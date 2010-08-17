/* 
 * File:   main.cpp
 * Author: kees
 *
 * Created on March 8, 2010, 2:29 PM
 */

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <stdint.h>

#include <sys/stat.h>
#include <dirent.h>

using namespace std;

#define POPPAK_MAGIC    (0xBAC04AC0)
#define POPPAK_VERSION  (0x0)

// Base for all exceptions
class Exception
{
public:
    Exception() {}
    virtual     ~Exception() {}

    virtual const string    diag() const        { return "Exception"; }
private:
};

class BadMagicException : Exception
{
public:
    BadMagicException(int32_t magic) : Exception(), _magic(magic) {}

    virtual const string    diag() const;
private:
    int32_t         _magic;
};
const string BadMagicException::diag() const
{
    char buffer[100];
    sprintf(buffer, "Bad magic number: %08X", _magic);
    return buffer;
}

class BadVersionException : Exception
{
public:
    BadVersionException(int32_t version) : Exception(), _version(version) {}

    virtual const string    diag() const;
private:
    int32_t         _version;
};
const string BadVersionException::diag() const
{
    char buffer[100];
    sprintf(buffer, "Unexpected version: %d", _version);
    return buffer;
}

class FileCorruptionException : Exception
{
public:
    FileCorruptionException(const char * fname, long int pos) : Exception(), _fname(fname), _pos(pos) {}

    virtual const string    diag() const;
private:
    const char *    _fname;
    long int        _pos;
};
const string FileCorruptionException::diag() const
{
    char buffer[100];
    sprintf(buffer, "File corruption in \"%s\" at position %ld", _fname, _pos);
    return buffer;
}

class FilenameTooLongException : Exception
{
public:
    FilenameTooLongException(const string & name) : Exception(), _name(name) {}

    virtual const string    diag() const;
private:
    const string    _name;
};
const string FilenameTooLongException::diag() const
{
    char buffer[100];
    sprintf(buffer, "Filename too long: \"%s\"", _name.c_str());
    return buffer;
}

class FileNotFoundException : Exception
{
public:
    FileNotFoundException(const string & name) : Exception(), _name(name) {}

    virtual const string    diag() const;
private:
    const string    _name;
};
const string FileNotFoundException::diag() const
{
    char buffer[100];
    sprintf(buffer, "File not found: \"%s\"", _name.c_str());
    return buffer;
}

class OpenDirFailedException : Exception
{
public:
    OpenDirFailedException(const string & name) : Exception(), _name(name) {}

    virtual const string    diag() const;
private:
    const string    _name;
};
const string OpenDirFailedException::diag() const
{
    char buffer[100];
    sprintf(buffer, "Failed to open directory: \"%s\"", _name.c_str());
    return buffer;
}

class ErrorReadingFileException : Exception
{
public:
    ErrorReadingFileException(const string & name) : Exception(), _name(name) {}

    virtual const string    diag() const;
private:
    const string    _name;
};
const string ErrorReadingFileException::diag() const
{
    char buffer[100];
    sprintf(buffer, "Error reading file: \"%s\"", _name.c_str());
    return buffer;
}

class DirectoryDoesNotExistException : Exception
{
public:
    DirectoryDoesNotExistException(const string & name) : Exception(), _name(name) {}

    virtual const string    diag() const;
private:
    const string    _name;
};
const string DirectoryDoesNotExistException::diag() const
{
    char buffer[100];
    sprintf(buffer, "Directory does not exist: \"%s\"", _name.c_str());
    return buffer;
}

class PopPakFileInfo
{
public:
    PopPakFileInfo(time_t time, const string & name, long int size, long int pos, int64_t filetime);

    time_t          Time() const { return _time; }
    const string    Name() const { return _name; }
    long int        Size() const { return _size; }
    long int        Pos() const { return _pos; }
    uint64_t        Filetime() const { return _filetime; }
private:
    time_t          _time;
    const string    _name;
    long int        _size;
    long int        _pos;
    int64_t         _filetime;      // Windows FILETIME
};

class PopPak
{
public:
    PopPak(const char* name, const string& mode);
    void            printdir();
    void            add_to_pak(const string & name);
    void            extract(const string & dir);
    void            finish();
    void            close();
    uint8_t         readb();
    uint32_t        readl();
    uint64_t        readq();
    const string    readstr(uint8_t len);
    const uint8_t * readcontent(const PopPakFileInfo * info, int & len);
    void            writeb(uint8_t b);
    void            writel(uint32_t l);
    void            writeq(uint64_t q);
    void            writestr(const string & str);
    void            writebuf(uint8_t * buf, int len);
    time_t          filetime_to_unixtime(uint64_t time) const;

private:
    void            readinfos();

private:
    const string    _name;
    string          _mode;
    FILE *          _fp;
    long int        _dataoffset;
    vector<PopPakFileInfo*>     _fileinfos;
};

PopPakFileInfo::PopPakFileInfo(time_t time, const string& name, long int size, long int pos, int64_t filetime) :
        _time(time),
        _name(name),
        _size(size),
        _pos(pos),
        _filetime(filetime)
{
}

PopPak::PopPak(const char* name, const string& mode) :
        _name(name),
        _fp(0),
        _dataoffset(0),
        _fileinfos()
{
    if (mode == "rb") {
        _mode = "r";
    } else if (mode == "wb") {
        _mode = "w";
    } else {
        _mode = mode;
    }

    if (mode == "r") {
        _fp = fopen(_name.c_str(), "rb");
        if (_fp) {
            readinfos();
        }
        _dataoffset = ftell(_fp);
    }
    // Writing new pak file is done in finish()
}

uint8_t PopPak::readb()
{
    uint8_t     val;
    int         nr;
    long int    pos;
    pos = ftell(_fp);
    nr = fread(&val, sizeof(val), 1, _fp);
    if (nr != 1) {
        throw (Exception *)new FileCorruptionException(_name.c_str(), pos);
    }
    val = val ^ 0xF7;
    return val;
}

uint32_t PopPak::readl()
{
    uint32_t    val;
    int         nr;
    long int    pos;
    pos = ftell(_fp);
    nr = fread(&val, sizeof(val), 1, _fp);
    if (nr != 1) {
        throw (Exception *)new FileCorruptionException(_name.c_str(), pos);
    }
    val = val ^ 0xF7F7F7F7;
    return val;
}

uint64_t PopPak::readq()
{
    uint64_t    val;
    int         nr;
    long int    pos;
    pos = ftell(_fp);
    nr = fread(&val, sizeof(val), 1, _fp);
    if (nr != 1) {
        throw (Exception *)new FileCorruptionException(_name.c_str(), pos);
    }
    val = val ^ 0xF7F7F7F7F7F7F7F7ull;
    return val;
}

const string PopPak::readstr(uint8_t len)
{
    if (len == 0) {
        return string("");
    }
    char *      buf = new char[len + 1];
    int         nr;
    long int    pos;
    pos = ftell(_fp);
    nr = fread(buf, sizeof(buf[0]), len, _fp);
    if (nr != len) {
        throw (Exception *)new FileCorruptionException(_name.c_str(), pos);
    }
    for (int i = 0; i < len; i++) {
        buf[i] ^= 0xF7;
    }
    buf[len] = 0;
    string str(buf);
    delete [] buf;
    return str;
}

const uint8_t * PopPak::readcontent(const PopPakFileInfo * info, int & len)
{
    if (!info || info->Size() == 0) {
        return NULL;
    }
    len = info->Size();
    uint8_t *   buf = new uint8_t[len];
    int         nr;
    long int    pos;
    fseek(_fp, info->Pos() + _dataoffset, SEEK_SET);
    nr = fread(buf, sizeof(buf[0]), len, _fp);
    if (nr != len) {
        throw (Exception *)new FileCorruptionException(_name.c_str(), pos);
    }
    for (int i = 0; i < len; i++) {
        buf[i] ^= 0xF7;
    }
    return buf;
}

void PopPak::writeb(uint8_t b)
{
    b = b ^ 0xF7;
    fwrite(&b, sizeof(b), 1, _fp);
}

void PopPak::writel(uint32_t l)
{
    l = l ^ 0xF7F7F7F7;
    fwrite(&l, sizeof(l), 1, _fp);
}

void PopPak::writeq(uint64_t q)
{
    q = q ^ 0xF7F7F7F7F7F7F7F7ull;
    fwrite(&q, sizeof(q), 1, _fp);
}

void PopPak::writestr(const string & str)
{
    for (size_t i = 0; i < str.length(); i++) {
        uint8_t b = str[i];
        b = b ^ 0xF7;
        fwrite(&b, sizeof(b), 1, _fp);
    }
}

void PopPak::writebuf(uint8_t * buf, int len)
{
    for (int i = 0; i < len; i++) {
        uint8_t b = buf[i];
        b = b ^ 0xF7;
        fwrite(&b, sizeof(b), 1, _fp);
    }
}

// Convert filetime to unix time (seconds since 1970-jan-1)
// FILETIME - Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
// d1 = datetime.date(1601,1,1)
// d2 = datetime.date(1970,1,1)
// (d2 - d1).days * 24 * 60 * 60 => 11644473600L
time_t PopPak::filetime_to_unixtime(uint64_t time) const
{
    if (time == 0) {
        return 0;
    }
    // 100-nanosecond => seconds
    time = time / 10000000;

    // seconds-since-1601 => seconds-since-1970
    time = time - 11644473600ull;

    return (time_t)time;
}

static string convert_slashes(string name)
{
    for (size_t i = 0; i < name.length(); i++) {
        if (name[i] == '\\') {
            name[i] = '/';
        }
    }
    return name;
}

void PopPak::readinfos()
{
    fseek(_fp, 0L, SEEK_SET);
    uint32_t magic = readl();
    if (magic != POPPAK_MAGIC) {
        close();
        throw (Exception *)new BadMagicException(magic);
    }
    int32_t version = readl();
    if (version != POPPAK_VERSION) {
        close();
        throw (Exception *)new BadVersionException(version);
    }
    long int    pos = 0;
    while (1) {
        uint8_t  flags = readb();
        if (flags & 0x80) {
            break;
        }

        uint8_t namelength = readb();
        string name = readstr(namelength);
        name = convert_slashes(name);

        uint32_t size = readl();

        uint64_t filetime = readq();
        time_t time = filetime_to_unixtime(filetime);

        PopPakFileInfo *    info = new PopPakFileInfo(time, name, size, pos, filetime);
        _fileinfos.push_back(info);

        pos += size;
    }
    cout << "nr of files: " << _fileinfos.size() << endl;
}

void PopPak::finish()
{
    if (_fp) {
        // ???? Huh?
        fclose(_fp);
    }

    cout << "Creating new pak: " << _name.c_str() << endl;
    _fp = fopen(_name.c_str(), "wb");

    writel(POPPAK_MAGIC);
    writel(POPPAK_VERSION);

    // Write the contents of all files to the pak.
    // Do the xor thing.
    for (size_t i = 0; i < _fileinfos.size(); i++) {
        PopPakFileInfo *    info = _fileinfos[i];

        // Flags
        writeb(0);

        if (info->Name().length() > 255) {
            throw (Exception *)new FilenameTooLongException(info->Name());
        }
        writeb(info->Name().length());
        writestr(info->Name());

        writel(info->Size());

        writeq(info->Filetime());
    }
    // Terminate file info header
    writeb(0x80);

    // Write all files too
    for (size_t i = 0; i < _fileinfos.size(); i++) {
        PopPakFileInfo *    info = _fileinfos[i];

        FILE * fp = fopen(info->Name().c_str(), "rb");
        if (!fp) {
            throw (Exception *)new Exception();
            continue;
        }
        uint8_t *   buffer  = new uint8_t[info->Size()];
        long int fsize = (int)fread(buffer, sizeof(buffer[0]), info->Size(), fp);
        if (fsize != info->Size()) {
            throw (Exception *)new ErrorReadingFileException(info->Name());
        }
        writebuf(buffer, info->Size());
        delete [] buffer;
        fclose(fp);
    }

    fclose(_fp);
    _fp = 0;
}

static const string dirname(const string & fname)
{
    size_t pos = fname.rfind('/');
    if (pos != fname.npos) {
        return fname.substr(0, pos);
    }
    return string("");
}

static void mkdirs(const string & dir)
{
    string dir1 = dirname(dir);
    if (dir1 != "") {
        mkdirs(dir1);
    }
    // cout << "dir: " << dir << endl;
    mkdir(dir.c_str(), 0777);       // Ignore errors.
}

void PopPak::extract(const string & dir)
{
    for (size_t i = 0; i < _fileinfos.size(); i++) {
        PopPakFileInfo *    info = _fileinfos[i];
        const string fname(dir + "/" + info->Name());
        // Is directory of fname present? If not mkdir it.
        mkdirs(dirname(fname));
        FILE * fp = fopen(fname.c_str(), "wb");
        if (!fp) {
            // TODO. Error
            continue;
        }
        int len = 0;
        const uint8_t * buf = readcontent(info, len);
        if (buf) {
            fwrite(buf, sizeof(*buf), len, fp);
            delete [] buf;
        }
        fclose(fp);
    }
}

void PopPak::close()
{
    if (_fp && _mode == "w") {
        finish();
    }
    if (_fp) {
        fclose(_fp);
        _fp = 0;
    }
}

void PopPak::printdir()
{
    const char *    fmt1 = "%-46.46s %20.20s %12.12s";
    const char *    fmt2 = "%-46.46s %20.20s %12d";
    char buffer[100+1];
    char buffer2[21];
    sprintf(buffer, fmt1, "File Name", "Modified    ", "Size");
    cout << buffer << endl;
    for (size_t i = 0; i < _fileinfos.size(); i++) {
        PopPakFileInfo *    info = _fileinfos[i];
        time_t  t1 = info->Time();
        strftime(buffer2, 21, "%Y-%b-%d %H:%M:%S", gmtime(&t1));
        sprintf(buffer, fmt2, info->Name().c_str(), buffer2, info->Size());
        cout << buffer << endl;
    }
}

void PopPak::add_to_pak(const string & name)
{
    //cout << "Adding " << name << endl;
    struct stat s;
    int i = stat(name.c_str(), &s);
    if (i != 0) {
        throw (Exception *)new FileNotFoundException(name);
        return;
    }

    if ((s.st_mode & S_IFDIR) == S_IFDIR) {
        // List files in the dir

        struct dirent * dp;
        DIR *   d = opendir(name.c_str());
        if (!d) {
            throw (Exception *)new OpenDirFailedException(name);
            return;
        }
        while ((dp = readdir(d)) != NULL) {
            string dname(dp->d_name);
            if (dname == "." || dname == "..") {
                continue;
            }
            add_to_pak(name + "/" + dname);
        }
    } else if ((s.st_mode & S_IFREG) == S_IFREG
            || (s.st_mode & S_IFLNK) == S_IFLNK) {
        // Regular file. Add it to the fileinfos
        long int size = s.st_size;
        time_t time = s.st_mtime;

        PopPakFileInfo *    info = new PopPakFileInfo(time, name, size, -1, 0);
        _fileinfos.push_back(info);
    }
}

void usage()
{
    const char * txt = "\
Usage:\n\
    tuxpak -l zipfile.pak         # Show listing of a pak\n\
    tuxpak -e zipfile.pak target  # Extract pak into target dir\n\
    tuxpak -c zipfile.pak src ... # Create pak from sources\
";
    cout << txt << endl;
}
/*
 * 
 */
int main(int argc, char** argv)
{
    try {
        if (argc == 3 && string(argv[1]) == "-l") {
            // Show listing
            PopPak * zf = new PopPak(argv[2], "r");
            zf->printdir();
            zf->close();
        } else if (argc >= 4 && string(argv[1]) == "-c") {
            // Create new pak
            PopPak * zf = new PopPak(argv[2], "w");
            for (int i = 3; i < argc; i++) {
                zf->add_to_pak(argv[i]);
            }
            zf->finish();
            zf->close();
        } else if (argc == 4 && string(argv[1]) == "-e") {
            // Extract pak
            PopPak * zf = new PopPak(argv[2], "r");
            zf->extract(argv[3]);
            zf->close();
        } else {
            usage();
            return EXIT_FAILURE;
        }
    }
    catch (Exception * e) {
        cout << e->diag() << endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        cout << "Unknown exception" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
