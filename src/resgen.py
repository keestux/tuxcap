#!/bin/env python

import os
from xml.dom import minidom

class Res(object):
    def __init__(self, resid = '', prefix = ''):
        self.resid = resid
        self.prefix = prefix
        self.type = ''
        self.idtype = ''

    def __str__(self):
        return self.prefix + self.resid

class ImageRes(Res):
    def __init__(self, resid = '', prefix = ''):
        Res.__init__(self, resid, prefix)
        self.type = 'Image'
        self.idtype = 'Image*'

class FontRes(Res):
    def __init__(self, resid = '', prefix = ''):
        Res.__init__(self, resid, prefix)
        self.type = 'Font'
        self.idtype = 'Font*'

class SoundRes(Res):
    def __init__(self, resid = '', prefix = ''):
        Res.__init__(self, resid, prefix)
        self.type = 'Sound'
        self.idtype = 'int'

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
        self.idprefix = ''

    def parse(self, fpath = None):
        if fpath is not None:
            self.fpath = fpath
        dom = minidom.parse(self.fpath)
        root = dom.getElementsByTagName('ResourceManifest')
        nodes = root[0].getElementsByTagName('Resources')
        for node in nodes:
            group = self.parseResource(node)
            self.groups.append(group)

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
            elif subnode.tagName == 'Image':
                resid = subnode.getAttribute('id')
                res = ImageRes(resid, idprefix)
                group.images.append(res)
            elif subnode.tagName == 'Sound':
                resid = subnode.getAttribute('id')
                res = SoundRes(resid, idprefix)
                group.sounds.append(res)
        return group

    header = """#ifndef __%s__ \n#define __%s__\n\n"""
    def writeHeader(self, name = 'Res', namespace = 'Sexy'):
        fp = file(name + '.h', 'wb')
        guard = os.path.basename(name).upper()
        fp.write(ResGen.header % (guard, guard))
        fp.write('namespace Sexy {\n')
        fp.write('\tclass ResourceManager;\n')
        fp.write('\tclass Image;\n')
        fp.write('\tclass Font;\n')
        fp.write('}\n\n')
        fp.write('namespace %s {\n' % namespace)
        fp.write('\tusing Sexy::ResourceManager;\n')
        fp.write('\tusing Sexy::Image;\n')
        fp.write('\tusing Sexy::Font;\n')
        fp.write('\n')
	fp.write('\tImage* LoadImageById(ResourceManager *theManager, int theId);\n')
	fp.write('\tbool ExtractResourcesByName(ResourceManager *theManager,'
                 'const char *theName);\n\n');

        for group in self.groups:
            self.writeGroupHeader(fp, group);

        self.writeGroupId(fp)

        fp.write('} // namespace %s\n\n' % namespace)
        fp.write('#endif\n')
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
        for group in self.groups:
            allres = group.getAll()
            for res in allres:
                fp.write('\t\t%s_ID,\n' % res)
        fp.write('\t\tRESOURCE_ID_MAX\n')
        fp.write('\t};\n')
	fp.write("""
        Image* GetImageById(int theId);
	Font* GetFontById(int theId);
	int GetSoundById(int theId);

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
            fp.write("""\tif (strcmp(theName, "%s") == 0)\n""" % group.resid)
            fp.write("""\t\treturn Extract%sResources(theManager);\n""" % group.resid)

        fp.write("""
	return false;
}\n\n""")

    def writeCPPGIBSI(self, fp, namespace):
        fp.write(
"""%s::ResourceId %s::GetIdByStringId(const char *theStringId)
{
	typedef std::map<std::string,int> MyMap;
	static MyMap aMap;
	if (aMap.empty())
	{
		for(int i = 0; i < RESOURCE_ID_MAX; i++)
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
	fp.write('\tResourceManager &aMgr = *theManager;\n\n')
        fp.write('\ttry\n')
        fp.write('\t{\n')

        allres = group.fonts + group.images + group.sounds
        for res in allres:
            fp.write('\t\t%s = aMgr.Get%sThrow("%s");\n' % \
                     (res, res.type, res))

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

        for group in self.groups:
            for res in group.fonts + group.images + group.sounds:
                fp.write('\t&%s,\n' % res)

        fp.write('\tNULL\n')
        fp.write('};\n\n')

    def writeCPPGetResources(self, fp, namespace):
        fp.write(
"""Image* %(ns)s::LoadImageById(ResourceManager *theManager, int theId)
{
	return (*((Image**)gResources[theId]) = theManager->LoadImage(GetStringIdById(theId)));
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

static %(ns)s::ResourceId GetIdByVariable(const void *theVariable)
{
	typedef std::map<long, int> MyMap;
	static MyMap aMap;
	if (gNeedRecalcVariableToIdMap)
	{
		gNeedRecalcVariableToIdMap = false;
		aMap.clear();
		for(int i = 0; i < RESOURCE_ID_MAX; i++)
			aMap[*(long*)gResources[i]] = i;
	}

	MyMap::iterator anItr = aMap.find(*(long*)theVariable);
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
	long theSoundId = theSound;
	return GetIdByVariable((void*)theSoundId);
}\n\n""" % { 'ns': namespace })

        fp.write("""const char* %s::GetStringIdById(int theId)\n""" % namespace)
        fp.write("{\n")
	fp.write("\tswitch (theId)\n")
	fp.write("\t{\n")


        for group in self.groups:
            for res in group.fonts + group.images + group.sounds:
                fp.write('\t\tcase %s_ID:\n' % res)
                fp.write('\t\t\treturn "%s";\n' % res)
        fp.write('\t\tdefault:\n\t\t\tbreak;\n')

        fp.write("\t}\n\n")
        fp.write('\treturn "";\n')
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

