#ifndef _qcd_helper_lib_h
#define _qcd_helper_lib_h

#include <windows.h>
#include "QCDModDefs.h"
#include "QCDModInput.h"

extern QCDModInitIn QCDCallbacks;

// -------------------------------------

enum reader_mode {
    READ = 0,
    WRITE = 1,
    MODIFY = 2 // Read + Write
};

class reader {
private:
    char *errormsg;

protected:
    void set_error ( const char *str );

public:
    reader();
    virtual ~reader();
    virtual size_t read ( void *ptr, size_t size ) = 0;
    virtual size_t write ( void *ptr, size_t size ) = 0;
    virtual __int64 get_position() = 0;
    virtual __int64 get_length() = 0;
    virtual bool set_eof() { return false; }
    virtual bool seek ( __int64 offset ) = 0;
    virtual bool can_seek() { return false; }
	const char *get_error() { return (const char *)errormsg; }
};

// -------------------------------------

class reader_file : public reader {
private:
    volatile HANDLE fh;

public:
    reader_file ( HANDLE handle );
    reader_file ( const char *name, reader_mode mode = READ );
    reader_file ( const WCHAR *name, reader_mode mode = READ );
    ~reader_file();
    virtual size_t read ( void *ptr, size_t size );
    virtual size_t write ( void *ptr, size_t size );
    virtual __int64 get_position();
    virtual __int64 get_length();
    virtual bool set_eof();
    virtual bool seek ( __int64 offset );
    virtual bool can_seek ();
};

// -------------------------------------

bool IsUnicode();

reader_file* new_reader ( const char *path, reader_mode mode = READ );

char *string_utf8_from_ansi ( const char *str );
char *string_utf8_from_ucs2 ( const WCHAR *str );

// -------------------------------------

class file_info {
private:
    typedef struct {
        char *name, *value;
    } meta_entry;

    meta_entry *meta;
    int meta_count;
    double length, bitrate;
    unsigned int samplerate, channels, bitspersample;

	unsigned int mode; //add by shaohao

    void freeall();

public:
    file_info();
    ~file_info();

    // multiple fields of the same name ARE allowed.
	const int meta_get_count() const { return meta_count; }
	const char *meta_enum_name ( int n ) const { return (n < 0 || n >= meta_count) ? NULL : meta[n].name; }
	const char *meta_enum_value ( int n ) const { return (n < 0 || n >= meta_count) ? NULL : meta[n].value; }

    void meta_modify_value ( int n, const char *new_value );
    void meta_insert ( int index, const char *name, const char *value );
    void meta_add ( const char *name, const char *value );
    void meta_remove ( int n );
    void meta_remove_all();
    const int meta_get_idx ( const char *name, int num=0 );

	void set_length ( double t ) { length = t; }
	void set_bitrate ( double t ) { bitrate = t; }
	void set_samplerate ( int t ) { samplerate = t; }
	void set_channels ( int t ) { channels = t; }
	void set_bitspersample ( int t ) { bitspersample = t; }
	void set_mode (int t) { mode = t; }
	const double get_length() const { return length; }
	const double get_bitrate() const { return bitrate; }
	const unsigned int get_samplerate() const { return samplerate; }
	const unsigned int get_channels() const { return channels; }
	const unsigned int get_bitspersample() const { return bitspersample; }
	const unsigned int get_mode() const { return mode; }

    void reset();

    //helper stuff for setting meta
    void meta_add_n ( const char *name, int name_len, const char *value, int value_len );
    void meta_remove_field ( const char *name ); //removes ALL fields of given name
    void meta_set ( const char *name, const char *value ); //deletes all fields of given name (if any), then adds new one
    const char *meta_get ( const char *name, int num=0 );
    const int meta_get_count_by_name ( const char *name );

    double meta_get_float ( const char *name );
};

#endif //_qcd_helper_lib_h