#include "qcdhelper.h"
#include <math.h>


// misc ------------------------------------------------------------------------

bool IsUnicode()
{
    OSVERSIONINFOEX osvi;
    BOOL bOsVersionInfoEx;

    ZeroMemory ( &osvi, sizeof(OSVERSIONINFOEX) );
    osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEX);

    if ( !(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi)) ) {
        osvi.dwOSVersionInfoSize = sizeof ( OSVERSIONINFO );
        if ( !GetVersionEx((OSVERSIONINFO *)&osvi) ) return FALSE;
    }

    return (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);
}

char *qcd_get_proxy ( const char *url )
{
    char *proxy_url = NULL;

    bool port_80 = true;

    if ( url ) {
        const char *proto = strstr ( url, "://" );
        if ( proto ) url = (const char *)proto + 3;
        while ( *url ) {
            char ch = *url++;
            if ( ch == '/' ) break;
            if ( ch == ':' ) {
                char tmp[32];
                char *p = (char *)tmp;
                while ( *url >= '0' && *url <= '9' && p < (tmp+sizeof(tmp)-1) ) *p++ = *url++;
                *p++ = '\0';
                if ( atoi(tmp) != 80 ) port_80 = false;
                break;
            }
        }
    }

    ProxyInfo proxy;
    memset ( &proxy, 0, sizeof(proxy) );
    proxy.struct_size = sizeof ( proxy );

    if ( QCDCallbacks.Service(opGetProxyInfo, &proxy, 0, 0) ) {
        if ( !proxy.usePort80Only || port_80 ) {
            proxy_url = (char *)malloc ( sizeof(proxy.username) + sizeof(proxy.password) + sizeof(proxy.hostname) + 64 );
            if ( proxy_url ) {
                char *ptr = proxy_url;
                if ( proxy.username[0] ) {
                    ptr += wsprintf ( ptr, "%s", proxy.username );
                    if ( proxy.password[0] ) {
                        ptr += wsprintf ( ptr, ":%s", proxy.password );
                    }
                    ptr += wsprintf ( ptr, "@" );
                }
                if ( proxy.hostname[0] ) {
                    ptr += wsprintf ( ptr, "%s", proxy.hostname );
                    if ( proxy.port ) {
                        ptr += wsprintf ( ptr, ":%u", (unsigned int)proxy.port );
                    }
                }
            }
        }
    }

    return (char *)proxy_url;
}

reader_file* new_reader ( const char *path, reader_mode mode )
{
    if ( !path || (path && !*path) ) return NULL;

    reader_file* new_r;
/*    if ( strstr(path, "://") ) {
        char *proxy_url = qcd_get_proxy ( path );
        new_r = new reader_http ( path, proxy_url );
        if ( proxy_url ) free ( proxy_url );
    } else*/ {
        WCHAR *path_ucs2 = NULL;
        if ( IsUnicode() ) {
            int size = strlen ( path ) + 1;
            WCHAR *path_ucs2 = (WCHAR *)malloc ( size * sizeof(WCHAR) );
            if ( path_ucs2 ) {
                path_ucs2[0] = L'\0';
                QCDCallbacks.Service ( opUTF8toUCS2, (void *)path, (long)path_ucs2, (long)size );
                new_r = new reader_file ( path_ucs2, mode );
                free ( path_ucs2 );
            } else {
                new_r = new reader_file ( path, mode );
            }
        } else {
            new_r = new reader_file ( path, mode );
        }
    }

    return new_r;
}

char *string_utf8_from_ansi ( const char *str )
{
    size_t size = strlen(str) + 1;
    WCHAR *wide = (WCHAR *)malloc ( size * sizeof(WCHAR) );
    char *utf8 = NULL;

    if ( wide ) {
        size = MultiByteToWideChar ( CP_ACP, 0, str, -1, wide, size );
        if ( size ) {
            size *= 6;
            utf8 = (char *)malloc ( size );
            if ( utf8 ) {
                QCDCallbacks.Service ( opUCS2toUTF8, (void *)wide, (long)utf8, (long)size );
            }
        }
        free ( wide );
    }

    return utf8;
}

char *string_utf8_from_ucs2(const WCHAR* pUCS2)
{
    char* pUTF8 = 0;
	int nSize = lstrlenW(pUCS2) + 1;

	if (nSize > 0) {
		pUTF8 = (char*)malloc(nSize * sizeof(WCHAR));
		if (pUTF8)
			QCDCallbacks.Service(opUCS2toUTF8, (void *)pUCS2, (long)pUTF8, (long)nSize);
	}
	return pUTF8;
}

static double pfc_string_to_float(const char * src)
{
    bool neg = false;
    __int64 val = 0;
    int div = 0;
    bool got_dot = false;

    while(*src==' ') src++;

    if (*src=='-') {neg = true;src++;}
    else if (*src=='+') src++;
    
    while(*src)
    {
        if (*src>='0' && *src<='9')
        {
            int d = *src - '0';
            val = val * 10 + d;
            if (got_dot) div--;
            src++;
        }
        else if (*src=='.' || *src==',')
        {
            if (got_dot) break;
            got_dot = true;
            src++;
        }
        else if (*src=='E' || *src=='e')
        {
            src++;
            div += atoi(src);
            break;
        }
        else break;
    }
    if (neg) val = -val;
    return (double) val * pow(10.0,(double)div);
}

// reader ----------------------------------------------------------------------

reader::reader()
{
    errormsg = strdup ( "" );
}

reader::~reader()
{
    if ( errormsg ) free ( errormsg );
}

void reader::set_error ( const char *str )
{
    if ( errormsg ) free ( errormsg );
    if ( str ) {
        errormsg = strdup ( str );
    } else {
        errormsg = strdup ( "" );
    }
}

// reader (file) ---------------------------------------------------------------

reader_file::reader_file ( HANDLE handle )
	: fh( handle )
{
}

reader_file::reader_file ( const char *name, reader_mode mode )
	: fh ( INVALID_HANDLE_VALUE )
{
	switch ( mode ) {
    case READ:
    default:
        fh = CreateFileA ( name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        break;

    case WRITE:
        fh = CreateFileA ( name, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        break;

    case MODIFY:
        fh = CreateFileA ( name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        break;
    };
}

reader_file::reader_file ( const WCHAR *name, reader_mode mode )
	: fh ( INVALID_HANDLE_VALUE )
{
    switch ( mode ) {
    case READ:
    default:
        fh = CreateFileW ( name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        break;

    case WRITE:
        fh = CreateFileW ( name, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        break;

    case MODIFY:
        fh = CreateFileW ( name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        break;
    };
}

reader_file::~reader_file()
{
    if ( fh != INVALID_HANDLE_VALUE ) {
        CloseHandle ( fh );
        fh = INVALID_HANDLE_VALUE;
    }
}

size_t reader_file::read ( void *ptr, size_t size )
{
    DWORD bytesread;
    if ( !ReadFile (fh, ptr, size, &bytesread, NULL) ) return 0;
    return bytesread;
}

size_t reader_file::write ( void *ptr, size_t size )
{
    DWORD byteswritten;
    if ( !WriteFile (fh, ptr, size, &byteswritten, NULL) ) return 0;
    return byteswritten;
}

__int64 reader_file::get_position()
{
    LONG high = 0;
    DWORD low = SetFilePointer ( fh, 0, &high, FILE_CURRENT );
    if ( (low == 0xFFFFFFFF) && (GetLastError() != NO_ERROR) ) return -1;
    return ((__int64)high << 32) | low;
}

__int64 reader_file::get_length()
{
    DWORD high = 0;
    DWORD low = GetFileSize ( fh, &high );
    if ( (low == INVALID_FILE_SIZE) && (GetLastError() != NO_ERROR) ) return -1;
    return ((__int64)high << 32) | low;
}

bool reader_file::set_eof()
{
    return SetEndOfFile ( fh ) ? true : false;
}

bool reader_file::seek ( __int64 offset )
{
    LONG high = (LONG)(offset >> 32);
    DWORD low = SetFilePointer ( fh, (LONG)(offset & (__int64)0xFFFFFFFF), &high, FILE_BEGIN );
    if ( (low == 0xFFFFFFFF) && (GetLastError() != NO_ERROR) ) return false;
    if ( (((__int64)high << 32) | low) == offset ) return true;
    return false;
}

bool reader_file::can_seek ()
{
    if ( GetFileType(fh) == FILE_TYPE_DISK ) return true;
    return false;
}


//-----------------------------------------------------------------------------
// file_info class
//-----------------------------------------------------------------------------

file_info::file_info()
	: meta(NULL)
	, meta_count(0)
	, length(0)
	, bitrate(0)
	, samplerate(0)
	, channels(0)
	, bitspersample(0)
	, mode(0)
{
}

file_info::~file_info()
{
    freeall();
}

void file_info::freeall()
{
    for ( int i = 0; i < meta_count; i++ ) {
        if ( meta[i].name ) free ( meta[i].name );
        if ( meta[i].value ) free ( meta[i].value );
    }

    if ( meta ) free ( meta );
    meta = NULL;
    meta_count = 0;
}

void file_info::meta_modify_value ( int n, const char *new_value )
{
    if ( n < 0 || n >= meta_count ) return;
    char *backup = meta[n].value;
    meta[n].value = (char *)realloc ( meta[n].value, strlen(new_value) + 1 );
    if ( !meta[n].value ) {
        meta[n].value = backup;
        return;
    }
    strcpy ( meta[n].value, new_value );
}

void file_info::meta_insert ( int index, const char *name, const char *value )
{
    void *backup = (void *)meta;
    if ( index < meta_count || index > meta_count ) index = meta_count;
    meta = (meta_entry *)realloc ( meta, sizeof(meta_entry) * (index + 1) );
    if ( !meta ) {
        meta = (meta_entry *)backup;
        return;
    }
    meta[index].name = (char *)malloc ( strlen(name) + 1 );
    if ( meta[index].name ) strcpy ( meta[index].name, name );
    meta[index].value = (char *)malloc ( strlen(value) + 1 );
    if ( meta[index].value ) strcpy ( meta[index].value, value );
    meta_count = index + 1;
}

void file_info::meta_add ( const char *name, const char *value )
{
    meta_insert ( meta_count, name, value );
}

void file_info::meta_remove ( int n )
{
    if ( n < 0 || n >= meta_count ) return;
    void *backup = (void *)meta;
    if ( meta[n].name ) free ( meta[n].name );
    if ( meta[n].value ) free ( meta[n].value );
    if ( n < meta_count - 1 ) {
        meta[n].name = meta[meta_count-1].name;
        meta[n].value = meta[meta_count-1].value;
    }
    if ( meta_count > 1 ) {
        meta = (meta_entry *)realloc ( meta, sizeof(meta_entry) * (meta_count-1) );
        if ( !meta ) meta = (meta_entry *)backup;
    } else {
        free ( meta );
        meta = NULL;
    }
    meta_count--;
}

void file_info::meta_remove_all()
{
    freeall();
}

const int file_info::meta_get_idx ( const char *name, int num )
{
    for ( int i = 0; i < meta_count; i++ ) {
        if ( meta[i].name && !stricmp(name, meta[i].name) ) {
            if ( num-- <= 0 ) return i;
        }
    }
    return -1;
}

void file_info::reset()
{
    meta_remove_all();
    set_length ( 0 );
}

void file_info::meta_add_n ( const char *name, int name_len, const char *value, int value_len )
{
    char *name_t = (char *)malloc ( name_len + 1 );
    if ( !name_t ) return;
    char *value_t = (char *)malloc ( value_len + 1 );
    if ( value_t ) {
        strncpy ( name_t, name, name_len );
        name_t[name_len] = '\0';
        strncpy ( value_t, value, value_len );
        value_t[value_len] = '\0';
        meta_add ( name_t, value_t );
        free ( value_t );
    }
    free ( name_t );
}

void file_info::meta_remove_field ( const char *name )
{
    for ( int i = 0; i < meta_get_count(); i++ ) {
        if ( meta[i].name && !stricmp(meta[i].name, name) ) meta_remove ( i );
    }
}

void file_info::meta_set ( const char *name, const char *value )
{
    meta_remove_field ( name );
    meta_add ( name, value );
}

const char *file_info::meta_get ( const char *name, int num )
{
    int idx = meta_get_idx ( name, num );
    if ( idx < 0 ) return 0;
    return meta_enum_value ( idx );
}

const int file_info::meta_get_count_by_name ( const char *name )
{
    int ret = 0;
    for ( int i = 0; i < meta_count; i++ ) {
        if ( meta[i].name && !stricmp(name, meta[i].name) ) ret++;
    }
    return ret;
}

double file_info::meta_get_float ( const char *name )
{
    const char *value = meta_get ( name, 0 );
    if ( !value || !*value ) return 0;
    return pfc_string_to_float ( value );
}
