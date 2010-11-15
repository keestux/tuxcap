#!/bin/env python

import os
from xml.dom import minidom

class Res(object):
    mytype = ''
    idtype = ''
    def __init__(self, resid='', prefix=''):
        self.resid = resid
        self.prefix = prefix

    def __str__(self):
        return self.prefix + self.resid

class ImageRes(Res):
    mytype = 'Image'
    idtype = 'Image*'

class FontRes(Res):
    mytype = 'Font'
    idtype = 'Font*'

class SoundRes(Res):
    mytype = 'Sound'
    idtype = 'int'

class ResGroup(object):
    def __init__(self, resid = ''):
        self.resid = resid
        self.images = []
        self.fonts = []
        self.sounds = []

    def getAll(self):
        return self.fonts + self.images + self.sounds

class ResGen(object):
    def __init__(self, fpath = 'resource.xml'):
        self.fpath = fpath
        self.groups = []
        self.allres = []
        self.idprefix = ''

    def parse(self, fpath = None):
        if fpath is not None:
            self.fpath = fpath
        groups = {}
        dom = minidom.parse(self.fpath)
        root = dom.getElementsByTagName('ResourceManifest')
        nodes = root[0].getElementsByTagName('Resources')
        for node in nodes:
            group = self.parseResource(node)
            print >> sys.stderr, "group: ", group.resid
            groups[group.resid] = group
        self.groups.append(groups['Game'])
        self.groups.append(groups['Init'])
        self.groups.append(groups['MainScreen'])
        self.groups.append(groups['TitleScreen'])

    def parseResource(self, node):
        idprefix = ''
        group = ResGroup(node.getAttribute('id'))
        for subnode in node.childNodes:
            if subnode.nodeType != minidom.Node.ELEMENT_NODE:
                continue
            if subnode.tagName == 'SetDefaults':
                if subnode.hasAttribute('idprefix'):
                    idprefix = subnode.getAttribute('idprefix')
            elif subnode.tagName == 'Font':
                resid = subnode.getAttribute('id')
                res = FontRes(resid, idprefix)
                group.fonts.append(res)
                self.allres.append(res)
            elif subnode.tagName == 'Image':
                resid = subnode.getAttribute('id')
                res = ImageRes(resid, idprefix)
                group.images.append(res)
                self.allres.append(res)
            elif subnode.tagName == 'Sound':
                resid = subnode.getAttribute('id')
                res = SoundRes(resid, idprefix)
                group.sounds.append(res)
                self.allres.append(res)

        group.fonts = sorted(group.fonts, key=lambda r: r.resid)
        group.images = sorted(group.images, key=lambda r: r.resid)
        group.sounds = sorted(group.sounds, key=lambda r: r.resid)
        return group

    header = """#ifndef __%s__ \n#define __%s__\n\n"""
    def writeHeader(self, name = 'Res', namespace = 'Sexy'):
        fp = file(name + '.h', 'wb')
        guard = name.capitalize() + '_H'
        fp.write(ResGen.header % (guard, guard))
        fp.write("""\
namespace Sexy
{
	class ResourceManager;
	class Image;
	class Font;
""")
        #fp.write('}\n\n')
        #fp.write('namespace %s {\n' % namespace)
        #fp.write('\tusing Sexy::ResourceManager;\n')
        #fp.write('\tusing Sexy::Image;\n')
        #fp.write('\tusing Sexy::Font;\n')
        fp.write("""
	Image* LoadImageById(ResourceManager *theManager, int theId);
	void ReplaceImageById(ResourceManager *theManager, int theId, Image *theImage);
	bool ExtractResourcesByName(ResourceManager *theManager, const char *theName);

""")

        for group in self.groups:
            self.writeGroupHeader(fp, group);

        self.writeGroupId(fp)

        fp.write("""
} // namespace %(ns)s


#endif
""" % {'ns': namespace})

        fp.close()

    def writeGroupHeader(self, fp, group):
        fp.write('\t// %s Resources\n' % group.resid)
        fp.write('\tbool Extract%sResources(ResourceManager *theMgr);\n' % group.resid)
        allres = group.getAll()
        for res in allres:
            fp.write('\textern %s %s;\n' % (res.idtype, res))
        if allres:
            fp.write('\n')

    def writeGroupId(self, fp):
        fp.write('\tenum ResourceId\n')
        fp.write('\t{\n')
        for res in self.allres:
            fp.write('\t\t%s_ID,\n' % res)
        fp.write('\t\tRESOURCE_ID_MAX\n')
        fp.write('\t};\n')
	fp.write("""
	Image* GetImageById(int theId);
	Font* GetFontById(int theId);
	int GetSoundById(int theId);

	Image*& GetImageRefById(int theId);
	Font*& GetFontRefById(int theId);
	int& GetSoundRefById(int theId);

	ResourceId GetIdByImage(Image *theImage);
	ResourceId GetIdByFont(Font *theFont);
	ResourceId GetIdBySound(int theSound);
	const char* GetStringIdById(int theId);
	ResourceId GetIdByStringId(const char *theStringId);\n""")

    def writeCPP(self, name = 'Res', namespace = 'Sexy'):
        fp = file(name + '.cpp', 'wb')
        fp.write('#include "%s.h"\n' % os.path.basename(name))
        fp.write('#include "ResourceManager.h"\n')
        fp.write('\n')
        fp.write('using namespace Sexy;\n')
        if namespace != 'Sexy':
            fp.write('using namespace %s;\n' % namespace)
        fp.write('\n')

        fp.write('static bool gNeedRecalcVariableToIdMap = false;\n\n');

        self.writeCPPERBN(fp, namespace)
        self.writeCPPGIBSI(fp, namespace)

        for group in self.groups:
            self.writeCPPGroup(fp, group, namespace)

        self.writeCPPResourceID(fp, namespace)
        self.writeCPPGetResources(fp, namespace)

        fp.close()

    def writeCPPERBN(self, fp, namespace):
        d = {'ns': namespace}
        fp.write("""bool %(ns)s::ExtractResourcesByName(ResourceManager *theManager, const char *theName)
{\n""" % d)
        for group in self.groups:
            fp.write("""\tif (strcmp(theName,"%s")==0)""" % group.resid)
            fp.write(""" return Extract%sResources(theManager);\n""" % group.resid)

        fp.write("""\
	return false;
}

""")

    def writeCPPGIBSI(self, fp, namespace):
        fp.write(
"""%s::ResourceId %s::GetIdByStringId(const char *theStringId)
{
	typedef std::map<std::string,int> MyMap;
	static MyMap aMap;
	if(aMap.empty())
	{
		for(int i=0; i<RESOURCE_ID_MAX; i++)
			aMap[GetStringIdById(i)] = i;
	}

	MyMap::iterator anItr = aMap.find(theStringId);
	if (anItr == aMap.end())
		return RESOURCE_ID_MAX;
	else
		return (ResourceId) anItr->second;
}\n\n"""  % (namespace, namespace))

    def writeCPPGroup(self, fp, group, namespace):
        fp.write('// %s Resources\n' % group.resid)
        allres = group.fonts + group.images + group.sounds
        for res in allres:
            fp.write('%s %s::%s;\n' % (res.idtype, namespace, res))
        if allres:
            fp.write('\n')


        fp.write("""bool %s::Extract%sResources(ResourceManager *theManager)\n""" % \
                 (namespace, group.resid))
        fp.write('{\n')
        fp.write('\tgNeedRecalcVariableToIdMap = true;\n\n')
	fp.write('\tResourceManager &aMgr = *theManager;\n')
        fp.write('\ttry\n')
        fp.write('\t{\n')

        allres = group.fonts + group.images + group.sounds
        for res in allres:
            fp.write('\t\t%s = aMgr.Get%sThrow("%s");\n' % \
                     (res, res.mytype, res))

        fp.write('\t}\n')
	fp.write('\tcatch(ResourceManagerException&)\n')
	fp.write('\t{\n')
        fp.write('\t\treturn false;\n')
        fp.write('\t}\n')
        fp.write('\treturn true;\n')
        fp.write('}\n\n')

    def writeCPPResourceID(self, fp, namespace):
        fp.write('static void* gResources[] =\n')
        fp.write('{\n')

        for res in self.allres:
            fp.write('\t&%s,\n' % res)

        fp.write('\tNULL\n')
        fp.write('};\n\n')

    def writeCPPGetResources(self, fp, namespace):
        fp.write("""\
Image* %(ns)s::LoadImageById(ResourceManager *theManager, int theId)
{
	return (*((Image**)gResources[theId]) = theManager->LoadImage(GetStringIdById(theId)));
}

void %(ns)s::ReplaceImageById(ResourceManager *theManager, int theId, Image *theImage)
{
	theManager->ReplaceImage(GetStringIdById(theId),theImage);
	*(Image**)gResources[theId] = theImage;
}

Image* %(ns)s::GetImageById(int theId)
{
	return *(Image**)gResources[theId];
}

Font* %(ns)s::GetFontById(int theId)
{
	return *(Font**)gResources[theId];
}

int %(ns)s::GetSoundById(int theId)
{
	return *(int*)gResources[theId];
}

Image*& %(ns)s::GetImageRefById(int theId)
{
	return *(Image**)gResources[theId];
}

Font*& %(ns)s::GetFontRefById(int theId)
{
	return *(Font**)gResources[theId];
}

int& %(ns)s::GetSoundRefById(int theId)
{
	return *(int*)gResources[theId];
}

static %(ns)s::ResourceId GetIdByVariable(const void *theVariable)
{
	typedef std::map<int,int> MyMap;
	static MyMap aMap;
	if(gNeedRecalcVariableToIdMap)
	{
		gNeedRecalcVariableToIdMap = false;
		aMap.clear();
		for(int i=0; i<RESOURCE_ID_MAX; i++)
			aMap[*(int*)gResources[i]] = i;
	}

	MyMap::iterator anItr = aMap.find((int)theVariable);
	if (anItr == aMap.end())
		return RESOURCE_ID_MAX;
	else
		return (ResourceId) anItr->second;
}

%(ns)s::ResourceId %(ns)s::GetIdByImage(Image *theImage)
{
	return GetIdByVariable(theImage);
}

%(ns)s::ResourceId %(ns)s::GetIdByFont(Font *theFont)
{
	return GetIdByVariable(theFont);
}

%(ns)s::ResourceId %(ns)s::GetIdBySound(int theSound)
{
	return GetIdByVariable((void*)theSound);
}

""" % { 'ns': namespace })

        fp.write("""const char* %s::GetStringIdById(int theId)\n""" % namespace)
        fp.write("{\n")
	fp.write("\tswitch(theId)\n")
	fp.write("\t{\n")

        for res in self.allres:
            fp.write('\t\tcase %s_ID:' % res)
            fp.write(' return "%s";\n' % res)
        fp.write('\t\tdefault: return "";\n')

        fp.write("\t}\n")
        #fp.write('\treturn "";\n')
        fp.write("}\n\n")

    def write(self, name = 'Res', namespace = 'Sexy'):
        self.writeHeader(name, namespace)
        self.writeCPP(name, namespace)

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 2:
        print 'USAGE: resource.xml'
        sys.exit(-1)

    resgen = ResGen()
    resgen.parse(sys.argv[1])
    if len(sys.argv) > 3:
        resgen.write(sys.argv[2], sys.argv[3])
    elif len(sys.argv) > 2:
        resgen.write(sys.argv[2])
    else:
        resgen.write()

