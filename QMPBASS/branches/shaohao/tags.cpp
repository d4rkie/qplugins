#include "tags.h"
#include "stdio.h"

// -------------------------------------
// tagging
// -------------------------------------

#define APE_TAG_FIELD_TITLE             "Title"
#define APE_TAG_FIELD_SUBTITLE          "Subtitle"
#define APE_TAG_FIELD_ARTIST            "Artist"
#define APE_TAG_FIELD_ALBUM             "Album"
#define APE_TAG_FIELD_DEBUTALBUM        "Debut Album"
#define APE_TAG_FIELD_PUBLISHER         "Publisher"
#define APE_TAG_FIELD_CONDUCTOR         "Conductor"
#define APE_TAG_FIELD_COMPOSER          "Composer"
#define APE_TAG_FIELD_COMMENT           "Comment"
#define APE_TAG_FIELD_YEAR              "Date"
#define APE_TAG_FIELD_RECORDDATE        "Record Date"
#define APE_TAG_FIELD_RECORDLOCATION    "Record Location"
#define APE_TAG_FIELD_TRACK             "Tracknumber"
#define APE_TAG_FIELD_GENRE             "Genre"
#define APE_TAG_FIELD_COVER_ART_FRONT   "Cover Art (front)"
#define APE_TAG_FIELD_NOTES             "Notes"
#define APE_TAG_FIELD_LYRICS            "Lyrics"
#define APE_TAG_FIELD_COPYRIGHT         "Copyright"
#define APE_TAG_FIELD_PUBLICATIONRIGHT  "Publicationright"
#define APE_TAG_FIELD_FILE              "File"
#define APE_TAG_FIELD_MEDIA             "Media"
#define APE_TAG_FIELD_EANUPC            "EAN/UPC"
#define APE_TAG_FIELD_ISRC              "ISRC"
#define APE_TAG_FIELD_RELATED_URL       "Related"
#define APE_TAG_FIELD_ABSTRACT_URL      "Abstract"
#define APE_TAG_FIELD_LANGUAGE          "Language"
#define APE_TAG_FIELD_BIBLIOGRAPHY_URL  "Bibliography"
#define APE_TAG_FIELD_BUY_URL           "Buy URL"
#define APE_TAG_FIELD_ARTIST_URL        "Artist URL"
#define APE_TAG_FIELD_PUBLISHER_URL     "Publisher URL"
#define APE_TAG_FIELD_FILE_URL          "File URL"
#define APE_TAG_FIELD_COPYRIGHT_URL     "Copyright URL"
#define APE_TAG_FIELD_INDEX             "Index"
#define APE_TAG_FIELD_INTROPLAY         "Introplay"
#define APE_TAG_FIELD_MJ_METADATA       "Media Jukebox Metadata"
#define APE_TAG_FIELD_DUMMY             "Dummy"

enum {
    MAX_FIELD_SIZE = 16*1024*1024 // treat bigger fields as errors
};

static const char*  ID3v1GenreList[] = {
    "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk",
    "Grunge", "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies",
    "Other", "Pop", "R&B", "Rap", "Reggae", "Rock",
    "Techno", "Industrial", "Alternative", "Ska", "Death Metal", "Pranks",
    "Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk",
    "Fusion", "Trance", "Classical", "Instrumental", "Acid", "House",
    "Game", "Sound Clip", "Gospel", "Noise", "AlternRock", "Bass",
    "Soul", "Punk", "Space", "Meditative", "Instrumental Pop", "Instrumental Rock",
    "Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk",
    "Eurodance", "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta",
    "Top 40", "Christian Rap", "Pop/Funk", "Jungle", "Native American", "Cabaret",
    "New Wave", "Psychadelic", "Rave", "Showtunes", "Trailer", "Lo-Fi",
    "Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical",
    "Rock & Roll", "Hard Rock", "Folk", "Folk/Rock", "National Folk", "Swing",
    "Fast-Fusion", "Bebob", "Latin", "Revival", "Celtic", "Bluegrass", "Avantgarde",
    "Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock", "Big Band",
    "Chorus", "Easy Listening", "Acoustic", "Humour", "Speech", "Chanson",
    "Opera", "Chamber Music", "Sonata", "Symphony", "Booty Bass", "Primus",
    "Porn Groove", "Satire", "Slow Jam", "Club", "Tango", "Samba",
    "Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle", "Duet",
    "Punk Rock", "Drum Solo", "A capella", "Euro-House", "Dance Hall",
    "Goa", "Drum & Bass", "Club House", "Hardcore", "Terror",
    "Indie", "BritPop", "NegerPunk", "Polsk Punk", "Beat",
    "Christian Gangsta", "Heavy Metal", "Black Metal", "Crossover", "Contemporary C",
    "Christian Rock", "Merengue", "Salsa", "Thrash Metal", "Anime", "JPop",
    "SynthPop",
};

static void insert_tag_field ( file_info *info, const char *field, const char *value )
{
    char *tmp = string_utf8_from_ansi ( value );
    if ( tmp ) {
        const char *pv = info->meta_get (field);
        if ( !pv || strlen (pv) < strlen (tmp) ) {
            info->meta_set ( field, tmp );
        }
        free ( tmp );
    }
}

// Get ID3v1 genre name
int GenreToString ( char* GenreStr, const int genre )
{
    if ( genre >= 0 && genre < sizeof (ID3v1GenreList) / sizeof (*ID3v1GenreList) ) {
        strcpy ( GenreStr, ID3v1GenreList[genre] );
        return 0;
    } else {
        GenreStr[0] = '\0';
        return 1;
    }
}

static void memcpy_crop ( void * _dst, const void * _src, size_t len )
{
    char * dst = (char*)_dst;
    const char * src = (const char*)_src;
    size_t i;

    for ( i = 0; i < len; i++ )
        if ( src[i] != '\0' )
            dst[i] = src[i];
        else
            break;

    // dst[i] points behind the string contents
    while ( i > 0 && dst[i-1] == ' ' )
        i--;

    dst[i] = '\0';
}

size_t strlen_max ( const char *p, size_t maxlen )
{
    if ( !p ) return 0;
    size_t len = 0;
    while ( p[len] && len < maxlen ) len++;
    return len;
}

struct APETagFooterStruct {
    char   ID       [8];
    char   Version  [4];
    char   Length   [4];
    char   TagCount [4];
    char   Flags    [4];
    char   Reserved [8];
};

static unsigned long Read_LE_Uint32_unsigned ( const unsigned char* p )
{
    return ((unsigned long)p[0] <<  0) |
           ((unsigned long)p[1] <<  8) |
           ((unsigned long)p[2] << 16) |
           ((unsigned long)p[3] << 24);
}

static unsigned long Read_LE_Uint32 ( const char* p ) {return Read_LE_Uint32_unsigned((const unsigned char*)p);}

static void Write_LE_Uint32 ( char* p, const unsigned long value )
{
    p[0] = (unsigned char) (value >>  0);
    p[1] = (unsigned char) (value >>  8);
    p[2] = (unsigned char) (value >> 16);
    p[3] = (unsigned char) (value >> 24);
}

struct Lyrics3TagFooterStruct {
    unsigned char   Length  [6];
    unsigned char   ID      [9];
};

struct Lyrics3TagField {
    unsigned char   ID      [3];
    unsigned char   Length  [5];
};

static int Lyrics3GetNumber5 ( const unsigned char* string )
{
    return ( string[0] - '0') * 10000 +
           ( string[1] - '0') * 1000 +
           ( string[2] - '0') * 100 +
           ( string[3] - '0') * 10 +
           ( string[4] - '0') * 1;
}

static int Lyrics3GetNumber6 ( const unsigned char* string )
{
    return ( string[0] - '0') * 100000 +
           ( string[1] - '0') * 10000 +
           ( string[2] - '0') * 1000 +
           ( string[3] - '0') * 100 +
           ( string[4] - '0') * 10 +
           ( string[5] - '0') * 1;
}

// Reads ID3v1.0 / ID3v1.1 tag
static int ReadID3v1Tag ( reader * fp, file_info * info, __int64 & tag_offset )
{
    unsigned char   tmp [128];
    char            value[32];

    if ( tag_offset < sizeof(tmp) ) return 0;
    if ( !fp->seek (tag_offset - sizeof(tmp)) ) return 0;
    if ( fp->read (tmp, sizeof (tmp)) != sizeof (tmp) ) return 0;
    if ( memcmp (tmp, "TAG", 3) ) return 0;

    if (info)
    {
        memcpy_crop ( value, tmp +  3, 30 );
        if ( value[0] != '\0' ) insert_tag_field (info, APE_TAG_FIELD_TITLE, value);
        memcpy_crop ( value, tmp + 33, 30 );
        if ( value[0] != '\0' ) insert_tag_field (info, APE_TAG_FIELD_ARTIST, value);
        memcpy_crop ( value, tmp + 63, 30 );
        if ( value[0] != '\0' ) insert_tag_field (info, APE_TAG_FIELD_ALBUM, value);
        memcpy_crop ( value, tmp + 93,  4 );
        if ( value[0] != '\0' ) insert_tag_field (info, APE_TAG_FIELD_YEAR, value);
        memcpy_crop ( value, tmp + 97, 30 );
        if ( value[0] != '\0' ) insert_tag_field (info, APE_TAG_FIELD_COMMENT, value);
        if ( tmp[125] == 0 && tmp[126] != 0 ) {
            sprintf ( value, "%d", tmp[126] );
            if ( value[0] != '\0' ) insert_tag_field (info, APE_TAG_FIELD_TRACK, value);
        }
        GenreToString ( value, tmp[127] );
        if ( value[0] != '\0' ) insert_tag_field (info, APE_TAG_FIELD_GENRE, value);
    }

    tag_offset -= sizeof (tmp);

    return 1;
}

// Reads Lyrics3 v2.0 tag
static int ReadLyrics3v2Tag ( reader * fp, file_info * info, __int64 & tag_offset )
{
    int                                 len;
    size_t                              size;
    struct Lyrics3TagFooterStruct       T;
    struct Lyrics3TagField              F;
    unsigned char                       tmpid3[128];
    char                                tmp[11];
    char                                value[32];
    char*                               tagvalue;

    if ( !fp->seek (tag_offset - 128) ) return 0;
    if ( fp->read (tmpid3, 128) != 128 ) return 0;
    // check for id3-tag
    if ( memcmp (tmpid3, "TAG", 3) ) return 0;
    if ( !fp->seek (tag_offset - 128 - sizeof (T)) ) return 0;
    if ( fp->read (&T, sizeof (T)) != sizeof (T) ) return 0;
    // check for lyrics3 v2.00 tag
    if ( memcmp (T.ID, "LYRICS200", sizeof (T.ID)) ) return 0;
    len = Lyrics3GetNumber6 (T.Length);
    if ( !fp->seek ( tag_offset - 128 - (int)sizeof (T) - len) ) return 0;
    if ( fp->read  (tmp, 11) != 11 ) return 0;
    if ( memcmp (tmp, "LYRICSBEGIN", 11) ) return 0;

    len -= 11; // header 'LYRICSBEGIN'

    while ( len > 0 ) {
        if ( fp->read (&F, 8) != 8 ) return 0;
        len -= 8;
        if ( (size = Lyrics3GetNumber5 (F.Length)) == 0 )
            continue;
        len -= size;
        if ( !(tagvalue = (char *)malloc (size + 1)) ) {
            return 0;
        }
        if ( fp->read (tagvalue, size) != (int)size ) {
            free (tagvalue);
            return 1;
        }
        tagvalue[size] = '\0';
        if ( tagvalue[0] == '\0' ) {
            free (tagvalue);
            continue;
        }

        if (info)
        {
            // Extended Title
            if ( !memcmp (F.ID, "ETT", 3) ) {
                if ( !memcmp (tagvalue, tmpid3 +  3, 30 ) ) {
                    insert_tag_field (info, APE_TAG_FIELD_TITLE, tagvalue);
                }
            } else
            // Extended Artist
            if ( !memcmp (F.ID, "EAR", 3) ) {
                if ( !memcmp (tagvalue, tmpid3 + 33, 30 ) ) {
                    insert_tag_field (info, APE_TAG_FIELD_ARTIST, tagvalue);
                }
            } else
            // Extended Album
            if ( !memcmp (F.ID, "EAL", 3) ) {
                if ( !memcmp (tagvalue, tmpid3 + 63, 30 ) ) {
                    insert_tag_field (info, APE_TAG_FIELD_ALBUM, tagvalue);
                }
            } else
            // Additional information
            if ( !memcmp (F.ID, "INF", 3) ) {
                insert_tag_field (info, APE_TAG_FIELD_COMMENT, tagvalue);
            } else
            // Lyrics
            if ( !memcmp (F.ID, "LYR", 3) ) {
                insert_tag_field (info, APE_TAG_FIELD_LYRICS, tagvalue);
            }
        }

        free (tagvalue);
    }

    if (info)
    {
        memcpy_crop ( value, tmpid3 +  3, 30 );
        if ( value[0] != '\0' ) insert_tag_field (info, APE_TAG_FIELD_TITLE, value);
        memcpy_crop ( value, tmpid3 + 33, 30 );
        if ( value[0] != '\0' ) insert_tag_field (info, APE_TAG_FIELD_ARTIST, value);
        memcpy_crop ( value, tmpid3 + 63, 30 );
        if ( value[0] != '\0' ) insert_tag_field (info, APE_TAG_FIELD_ALBUM, value);
        memcpy_crop ( value, tmpid3 + 93,  4 );
        if ( value[0] != '\0' ) insert_tag_field (info, APE_TAG_FIELD_YEAR, value);
        memcpy_crop ( value, tmpid3 + 97, 30 );
        if ( value[0] != '\0' ) insert_tag_field (info, APE_TAG_FIELD_COMMENT, value);
        if ( tmpid3[125] == 0 && tmpid3[126] != 0 ) {
            sprintf ( value, "%d", tmpid3[126] );
            if ( value[0] != '\0' ) insert_tag_field (info, APE_TAG_FIELD_TRACK, value);
        }
        GenreToString ( value, tmpid3[127] );
        if ( value[0] != '\0' ) insert_tag_field (info, APE_TAG_FIELD_GENRE, value);
    }

    tag_offset -= 128 + Lyrics3GetNumber6 (T.Length) + sizeof (T);

    return 0;
}

// Reads APE v1.0/2.0 tag
int ReadAPETag ( reader *fp, file_info *info, __int64 &tag_offset )
{
    unsigned long               vsize;
    unsigned long               isize;
    unsigned long               flags;
    unsigned long               remaining;
    char*                       buff;
    char*                       p;
    char*                       end;
    struct APETagFooterStruct   T;
    unsigned long               TagLen;
    unsigned long               TagCount;
    unsigned long               Ver;

    if ( tag_offset < sizeof (T) ) return 0;
    if ( !fp->seek (tag_offset - sizeof (T)) ) return 0;
    if ( fp->read (&T, sizeof T) != sizeof (T) ) return 0;
    if ( memcmp (T.ID, "APETAGEX", sizeof (T.ID)) ) return 0;
    Ver = Read_LE_Uint32 (T.Version);
    if ( (Ver != 1000) && (Ver != 2000) ) return 0;
    if ( (TagLen = Read_LE_Uint32 (T.Length)) < sizeof (T) ) return 0;

    if (info) {
        if ( !fp->seek (tag_offset - TagLen) ) return 0;
        if ( !(buff = (char *)malloc (TagLen)) ) return 0;
        if ( fp->read (buff, TagLen - sizeof (T)) != (int)(TagLen - sizeof (T)) ) {
            free (buff);
            return 0;
        }

        bool b_warning = false;

        char *name = NULL;
        char *value = NULL;

        TagCount = Read_LE_Uint32 (T.TagCount);
        end = buff + TagLen - sizeof (T);
        for ( p = buff; p < end && TagCount--; ) {
            if (end - p < 8) break;
            vsize = Read_LE_Uint32 (p); p += 4;
            flags = Read_LE_Uint32 (p); p += 4;

            remaining = (unsigned long)(end - p);

            isize = strlen_max (p, remaining);

            if (isize >= remaining || vsize > MAX_FIELD_SIZE || isize + 1 + vsize > remaining) break;//incorrect data

            name = (char *)malloc ( isize+1 );
            if ( !name ) {
                free ( buff );
                return 0;
            }

            value = (char *)malloc ( vsize+1 );
            if ( !value ) {
                free ( name );
                free ( buff );
                return 0;
            }

            memcpy ( name, p, isize );
            name[isize] = '\0';
            memcpy ( value, p+isize+1, vsize );
            value[vsize] = '\0';

            if ( isize > 0 && vsize > 0 ) {
                if ( Ver == 2000 ) {
                    if ( !(flags & 1<<1) ) {    // insert UTF-8 string
                        info->meta_remove_field(name);

                        info->meta_add ( name, value );

                        {
                            const char *p = value;
                            const char *pe = p + vsize;

                            for(;;) {
                                p += strlen_max(p,pe-p) + 1;
                                if (p>=pe) break;
                                info->meta_add ( name, p );
                            }
                        }
                    } else {                    // insert binary data
                        char *tmp = string_utf8_from_ansi ( value );
                        if ( tmp ) {
                            info->meta_set ( name, tmp );
                            free ( tmp );
                        }
                    }
                } else {
                    char *tmp = string_utf8_from_ansi ( value );
                    if ( tmp ) {
                        info->meta_set ( name, tmp );
                        free ( tmp );
                    }
                }
            }
            p += isize + 1 + vsize;
            free ( name );
            free ( value );
        }

        free ( buff );
    }

    tag_offset -= TagLen;

    if ( Read_LE_Uint32 (T.Flags) & (1<<31) )   // Tag contains header
        tag_offset -= sizeof (T);

    return 1;
}

static int WriteAPE2Tag ( reader* fp, file_info *info )
{
    char                        temp[4];
    struct APETagFooterStruct   T;
    unsigned int                flags;
    int                         TagCount;
    int                         TagSize;
    int                         i;

    TagCount = 0;

    TagSize = sizeof(T);                                // calculate size of buffer needed

    char *tag_data = (char *)malloc ( sizeof(T) );
    if ( !tag_data ) return 0;
    size_t tag_data_pos = sizeof(T);

    for ( i = 0; i < info->meta_get_count() ; i++ ) {
        const char *name = info->meta_enum_name(i);

        int first = info->meta_get_idx(name);
        if ( first < i ) continue;

        size_t v_len = strlen ( info->meta_enum_value(i) );
        char *value = (char *)malloc ( v_len + 1 );
        if ( !value ) {
            if ( tag_data ) free ( tag_data );
            return 0;
        }
        memcpy ( value, info->meta_enum_value(i), v_len );

        {
            int num = info->meta_get_count_by_name (name);
            if ( num > 1 ) {
                for ( int z = 1; z < num; z++ ) {
                    const char *src = info->meta_get(name, z);
                    if ( src && *src ) {
                        size_t last_len = v_len;
                        size_t src_len = strlen(src);
                        v_len += 1 + src_len;
                        void *backup = (void *)value;
                        value = (char *)realloc ( value, v_len + 1 );
                        if ( !value ) {
                            free ( backup );
                            return 0;
                        }
                        value[last_len] = '\0';
                        memcpy ( value+last_len+1, src, src_len );
                    }
                }
            }
        }

        size_t n_len = strlen(name);
        void *backup = (void *)tag_data;
        tag_data = (char *)realloc ( tag_data, tag_data_pos + 4+4+n_len+1+v_len + sizeof(T) );
        if ( !tag_data ) {
            free ( value );
            if ( backup ) free ( backup );
            return 0;
        }
        Write_LE_Uint32(temp, v_len);
        memcpy ( tag_data+tag_data_pos, temp, 4 ); tag_data_pos += 4;
        Write_LE_Uint32(temp, 0);
        memcpy ( tag_data+tag_data_pos, temp, 4 ); tag_data_pos += 4;
        memcpy ( tag_data+tag_data_pos, name, n_len+1 ); tag_data_pos += n_len+1;
        memcpy ( tag_data+tag_data_pos, value, v_len ); tag_data_pos += v_len;
        TagCount++;
        free ( value );
    }

    if (TagCount == 0) {
        if ( tag_data ) free ( tag_data );
        return fp->set_eof() ? 1 : 0;
    }

    TagSize = tag_data_pos;

    char *p = (char *)tag_data;

    flags  = 1<<31;                                     // contains header
    flags |= 1<<29;                                     // this is the header
    memcpy(T.ID, "APETAGEX", sizeof(T.ID));             // ID String
    Write_LE_Uint32(T.Version, 2000);                   // Version 2.000
    Write_LE_Uint32(T.Length, TagSize);                 // Tag size
    Write_LE_Uint32(T.TagCount, TagCount);              // Number of fields
    Write_LE_Uint32(T.Flags, flags);                    // Flags
    memset(T.Reserved, 0, sizeof(T.Reserved));          // Reserved
    memcpy(p, &T, sizeof(T)); p += sizeof(T);           // insert header

    p += tag_data_pos - sizeof(T);

    flags  = 1<<31;                                     // contains header
    memcpy(T.ID, "APETAGEX", sizeof(T.ID));             // ID String
    Write_LE_Uint32(T.Version, 2000);                   // Version 2.000
    Write_LE_Uint32(T.Length, TagSize);                 // Tag size - header
    Write_LE_Uint32(T.TagCount, TagCount);              // Number of fields
    Write_LE_Uint32(T.Flags, flags);                    // Flags
    memset(T.Reserved, 0, sizeof(T.Reserved));          // Reserved
    memcpy(p, &T, sizeof(T));                           // insert footer

    int ret = 0;
    if ( fp->write(tag_data, TagSize + sizeof(T)) == TagSize + sizeof(T) ) {
        if ( fp->set_eof() ) ret = 1;
    }

    free ( tag_data );

    return ret;
}

bool read_tags ( reader *r, file_info *info )
{
    bool success = false;
    __int64 tag_offset = r->get_length();
    __int64 offs_bk;
    r->seek ( tag_offset );

    do {
        offs_bk = tag_offset;
        if ( ReadAPETag(r, info, tag_offset) ) success = true;
        if ( ReadLyrics3v2Tag(r, info, tag_offset) ) success = true;
        if ( ReadID3v1Tag(r, info, tag_offset) ) success = true;
    } while ( offs_bk != tag_offset );

    return success;
}

bool seek_to_tag_offset ( reader *r )
{
    __int64 tag_offset = r->get_length();
    __int64 offs_bk;

    if ( !r->seek(tag_offset) ) return false;

    do {
        offs_bk = tag_offset;
        ReadAPETag ( r, NULL, tag_offset );
        ReadLyrics3v2Tag ( r, NULL, tag_offset );
        ReadID3v1Tag ( r, NULL, tag_offset );
    } while ( offs_bk != tag_offset );

    return true;
}
