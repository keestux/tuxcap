#!/bin/env python

import os
from xml.dom import minidom

class Res(object):
    mytype = ''
    idtype = ''
    def __init__(self, resid='', prefix='', path=''):
        self.resid = resid
        self.path = path
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
    def __init__(self, options, fpath='resource.xml'):
        self.options = options
        self.fpath = fpath
        self.groups = []
        self.allres = []
        self.allresid = {}
        self.idprefix = ''

    def parse(self, fpath=None):
        if fpath is not None:
            self.fpath = fpath
        dom = minidom.parse(self.fpath)
        root = dom.getElementsByTagName('ResourceManifest')
        nodes = root[0].getElementsByTagName('Resources')
        for node in nodes:
            group = self.parseResource(node)
            if self.options.verbose:
                print("group: " + group.resid)
            self.groups.append(group)

    def appendRes(self, res):
        if res.resid in self.allresid:
            # Only accept duplicates if path is same too
            if res.path != self.allresid[res.resid].path:
                import sys
                print >> sys.stderr, "ERROR: Resources must have unique path"
                print >> sys.stderr, " new\t", res.resid, res.path
                print >> sys.stderr, " old\t", res.resid, self.allresid[res.resid].path
                sys.exit(1)
        else:
            self.allres.append(res)
            self.allresid[res.resid] = res

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
                path = subnode.getAttribute('path')
                res = FontRes(resid, idprefix, path)
                group.fonts.append(res)
                self.appendRes(res)
            elif subnode.tagName == 'Image':
                resid = subnode.getAttribute('id')
                path = subnode.getAttribute('path')
                res = ImageRes(resid, idprefix, path)
                group.images.append(res)
                self.appendRes(res)
            elif subnode.tagName == 'Sound':
                resid = subnode.getAttribute('id')
                path = subnode.getAttribute('path')
                res = SoundRes(resid, idprefix, path)
                group.sounds.append(res)
                self.appendRes(res)

        group.fonts = sorted(group.fonts, key=lambda r: r.resid)
        group.images = sorted(group.images, key=lambda r: r.resid)
        group.sounds = sorted(group.sounds, key=lambda r: r.resid)
        return group

    header = """#ifndef __%s__ \n#define __%s__\n\n"""
    def writeHeader(self, name='Res', namespace='Sexy'):
        if self.options.verbose:
            print("writeHeader('%(name)s', '%(namespace)s')" % vars())
        fp = file(name + '.h', 'wb')
        guard = name.capitalize() + '_H'
        fp.write(ResGen.header % (guard, guard))
        fp.write("""\
namespace Sexy
{
	class ResourceManager;
	class Image;
	class Font;
}
""")
        fp.write("""
Sexy::Image* LoadImageById(Sexy::ResourceManager *theManager, int theId);
void ReplaceImageById(Sexy::ResourceManager *theManager, int theId, Sexy::Image *theImage);
bool ExtractResourcesByName(Sexy::ResourceManager *theManager, const char *theName);

""")

        for group in self.groups:
            self.writeGroupHeader(fp, group);

        self.writeGroupId(fp)

        fp.write("""

#endif
""")

        fp.close()

    def writeGroupHeader(self, fp, group):
        fp.write('// %s Resources\n' % group.resid)
        fp.write('bool Extract%sResources(Sexy::ResourceManager *theMgr);\n' % group.resid)
        allres = group.getAll()
        for res in allres:
            if res.idtype == 'int':
                fp.write('extern %s %s;\n' % (res.idtype, res))
            else:
                fp.write('extern Sexy::%s %s;\n' % (res.idtype, res))
        if allres:
            fp.write('\n')

    def writeGroupId(self, fp):
        fp.write('enum ResourceId\n')
        fp.write('{\n')
        for res in self.allres:
            fp.write('\t%s_ID,\n' % res)
        fp.write('\tRESOURCE_ID_MAX\n')
        fp.write('};\n')

        fp.write("""
Sexy::Image* GetImageById(int theId);
Sexy::Font* GetFontById(int theId);
int GetSoundById(int theId);

Sexy::Image*& GetImageRefById(int theId);
Sexy::Font*& GetFontRefById(int theId);
int& GetSoundRefById(int theId);

ResourceId GetIdByImage(Sexy::Image *theImage);
ResourceId GetIdByFont(Sexy::Font *theFont);
ResourceId GetIdBySound(int theSound);
const char* GetStringIdById(int theId);
ResourceId GetIdByStringId(const char *theStringId);\n""")

    def writeCPP(self, name='Res', namespace='Sexy'):
        fp = file(name + '.cpp', 'wb')
        fp.write('#include "%s.h"\n' % os.path.basename(name))
        fp.write('#include "ResourceManager.h"\n')
        fp.write('\n')
        fp.write('using namespace Sexy;\n')
        if namespace and namespace != 'Sexy':
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

    # ERBN => ExtractResourceByName
    def writeCPPERBN(self, fp, namespace):
        d = {}
        if namespace:
            d['ns'] = namespace + '::'
        else:
            d['ns'] = ''
        fp.write("""\
bool %(ns)sExtractResourcesByName(ResourceManager *theManager, const char *theName)
{
""" % d)
        for group in self.groups:
            d['resid'] = group.resid
            fp.write("""\
	if (strcmp(theName,"%(resid)s")==0) return Extract%(resid)sResources(theManager);
""" % d)

        fp.write("""\
	return false;
}

""")

    # GIBSI => GetIdByStringId
    def writeCPPGIBSI(self, fp, namespace):
        d = {}
        if namespace:
            d['ns'] = namespace + '::'
        else:
            d['ns'] = ''
        fp.write("""\
%(ns)sResourceId %(ns)sGetIdByStringId(const char *theStringId)
{
	typedef std::map<std::string,int> MyMap;
	static MyMap aMap;
	if (aMap.empty())
	{
		for (int i = 0; i < RESOURCE_ID_MAX; i++)
			aMap[GetStringIdById(i)] = i;
	}

	MyMap::iterator anItr = aMap.find(theStringId);
	if (anItr == aMap.end())
		return RESOURCE_ID_MAX;
	else
		return (ResourceId) anItr->second;
}

"""  % d)

    def writeCPPGroup(self, fp, group, namespace):
        d = {}
        d['resid'] = group.resid
        if namespace:
            d['ns'] = namespace + '::'
        else:
            d['ns'] = ''


        fp.write("""\
bool %(ns)sExtract%(resid)sResources(ResourceManager *theManager)
{
	gNeedRecalcVariableToIdMap = true;

	ResourceManager &aMgr = *theManager;
	try
	{
""" % d)

        allres = group.fonts + group.images + group.sounds
        for res in allres:
            d['res'] = res
            d['mytype'] = res.mytype
            fp.write('\t\t%(res)s = aMgr.Get%(mytype)sThrow("%(res)s");\n' % d)

        fp.write("""\
	}
	catch(ResourceManagerException&)
	{
		return false;
	}
	return true;
}

""")

    def writeCPPResourceID(self, fp, namespace):
        d = {}
        if namespace:
            d['ns'] = namespace + '::'
        else:
            d['ns'] = ''

        fp.write('// Resources\n' % d)
        for res in self.allres:
            d['restype'] = res.idtype
            d['res'] = res
            if res.idtype == 'int':
                fp.write('%(restype)s %(ns)s%(res)s;\n' % d)
            else:
                fp.write('Sexy::%(restype)s %(ns)s%(res)s;\n' % d)
        if self.allres:
            fp.write('\n')

        fp.write("""\
static void* gResources[] =
{
""")
        for res in self.allres:
            d['res'] = res
            fp.write("""\
	&%(res)s,
""" % d)
        fp.write('\tNULL\n')
        fp.write('};\n\n')

    def writeCPPGetResources(self, fp, namespace):
        d = {}
        if namespace:
            d['ns'] = namespace + '::'
        else:
            d['ns'] = ''

        fp.write("""\
Image* %(ns)sLoadImageById(ResourceManager *theManager, int theId)
{
	return (*((Image**)gResources[theId]) = theManager->LoadImage(GetStringIdById(theId)));
}

void %(ns)sReplaceImageById(ResourceManager *theManager, int theId, Image *theImage)
{
	theManager->ReplaceImage(GetStringIdById(theId),theImage);
	*(Image**)gResources[theId] = theImage;
}

Image* %(ns)sGetImageById(int theId)
{
	return *(Image**)gResources[theId];
}

Font* %(ns)sGetFontById(int theId)
{
	return *(Font**)gResources[theId];
}

int %(ns)sGetSoundById(int theId)
{
	return *(int*)gResources[theId];
}

Image*& %(ns)sGetImageRefById(int theId)
{
	return *(Image**)gResources[theId];
}

Font*& %(ns)sGetFontRefById(int theId)
{
	return *(Font**)gResources[theId];
}

int& %(ns)sGetSoundRefById(int theId)
{
	return *(int*)gResources[theId];
}

static %(ns)sResourceId GetIdByVariable(const void *theVariable)
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

%(ns)sResourceId %(ns)sGetIdByImage(Image *theImage)
{
	return GetIdByVariable(theImage);
}

%(ns)sResourceId %(ns)sGetIdByFont(Font *theFont)
{
	return GetIdByVariable(theFont);
}

%(ns)sResourceId %(ns)sGetIdBySound(int theSound)
{
	return GetIdByVariable((void*)theSound);
}

""" % d)

        fp.write("""\
const char* %(ns)sGetStringIdById(int theId)
{
	switch (theId)
	{
""" % d)

        for res in self.allres:
            d['res'] = res
            fp.write("""\
	case %(res)s_ID: return "%(res)s";
""" % d)
        fp.write('\tdefault: return "";\n')

        fp.write("\t}\n")
        #fp.write('\treturn "";\n')
        fp.write("}\n\n")

    def write(self, name='Res', namespace='Sexy'):
        self.writeHeader(name, namespace)
        self.writeCPP(name, namespace)

def main():
    from optparse import OptionParser
    parser = OptionParser(usage='usage: %prog [options] resource_file(s)', version="%prog 0.4")
    parser.add_option("-v", "--verbose",
        action="store_true", default=False, dest='verbose',
        help="Give verbose output.")
    parser.add_option("-n", "--namespace",
        default="Sexy", dest='namespace',
        metavar="MODULE", help="namespace (default %default)")
    parser.add_option("-m", "--module",
        default="Res", dest='module',
        metavar="RES",help="name of the C++ module (default %default)")

    options, args = parser.parse_args()
    if len(args) < 1:
        parser.error("incorrect number of arguments")
        return

    resgen = ResGen(options)
    for a in args:
        if options.verbose:
            print "Parsing " + a
        resgen.parse(a)

    resgen.write(options.module, options.namespace)

if __name__ == '__main__':
    import sys
    try:
        main()
    except SystemExit, e:
        pass
    except Exception, e:
        print >> sys.stderr, e
        sys.exit(1)
