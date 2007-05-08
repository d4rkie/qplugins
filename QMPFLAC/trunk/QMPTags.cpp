#include "stdafx.h"
#include "QMPFLAC.h"
#include "QMPTags.h"

#include "FLAC++/all.h"

#include "AboutDlg.h"

//..............................................................................
// Global Variables

QCDModInitTag2	QMPTags::QCDCallbacks;


//-----------------------------------------------------------------------------

// IO callbacks for read/write/strip
::FLAC__IOCallbacks iocb = {
	(::FLAC__IOCallback_Read)fread, 
	(::FLAC__IOCallback_Write)fwrite, 
	(::FLAC__IOCallback_Seek)_fseeki64, 
	(::FLAC__IOCallback_Tell)_ftelli64, 
	(::FLAC__IOCallback_Eof)feof, 
	NULL
};

bool _strip_all_tags(FLAC::Metadata::Iterator & iterator);

//-----------------------------------------------------------------------------

BOOL QMPTags::Initialize(QCDModInfo *modInfo, int flags)
{
	modInfo->moduleString = (char *)L"FLAC Tags";
	modInfo->moduleExtensions = (char *)L"FLAC:FLA";

	return TRUE;
}

//-----------------------------------------------------------------------------

void QMPTags::ShutDown(int flags)
{
	// TODO:
	// prepare plugin to be unloaded. All allocations should be freed.
}

//-----------------------------------------------------------------------------

BOOL QMPTags::ReadFromFile(LPCWSTR filename, void* tagHandle, int flags)
{
	FILE * fp;
	FLAC::Metadata::Chain chain;
	FLAC::Metadata::Iterator iterator;

	// open the file and read all of its metadata
	fp = _wfopen( filename, L"rb");
	if ( !fp) return FALSE;
	if ( !chain.read( fp, iocb, false)) {
		fclose( fp);
		return FALSE;
	}

	// process the read data
	if ( !iterator.is_valid()) {
		fclose( fp);
		return FALSE;
	}
 
	// read, close it.
	fclose( fp);

	// initialize the iterator
	iterator.init( chain);

	// enumerate all metadata with iterator
	int index = 0;
	do {
		switch ( iterator.get_block_type())
		{
		case FLAC__METADATA_TYPE_VORBIS_COMMENT: // for common string tags
			{
				unsigned vcindex;
				unsigned num;

				// prepare and initialize VorbisComment metadata instance
				FLAC::Metadata::VorbisComment * vcdata = dynamic_cast< FLAC::Metadata::VorbisComment * >(iterator.get_block());
				if ( !vcdata)
					return FALSE;

				if ( FLAC__METADATA_TYPE_VORBIS_COMMENT != vcdata->get_type()) {
					delete vcdata;
					return FALSE;
				}

				// start reading & processing metadata
				num = vcdata->get_num_comments();
				for ( vcindex = 0; vcindex < num; ++vcindex) {
					FLAC::Metadata::VorbisComment::Entry entry = vcdata->get_comment( vcindex);

					// get field value
					unsigned bvl = entry.get_field_value_length();
					if ( bvl == 0) continue; // skip the ZERO length comment.
					else bvl++; // add NULL tailing
					LPBYTE bv = new BYTE[bvl]; ZeroMemory( bv, sizeof(BYTE)*bvl);
					memcpy( bv, entry.get_field_value(),bvl-1);

					// get field name
					CStringW wn;
					UTF8toUCS2( entry.get_field_name(), wn);
					wn.MakeUpper();

					// start processing vorbis-comment.
					if ( wn == QCDTag_Comment) { // for comment tag-group
						DWORD cl = 0, dl = 0;
						CStringW c, d;
						QTD_STRUCT_COMMENT comment; ZeroMemory( &comment, sizeof(QTD_STRUCT_COMMENT));

						// get comment
						cl = lstrlenA( (LPCSTR)bv) + 1;
						UTF8toUCS2( bv, c);
						comment.pwszComment = (LPWSTR)(LPCWSTR)c;
						// get description
						if ( bvl - cl > 0) {
							dl = lstrlenA( (LPCSTR)(bv+cl)) + 1;
							UTF8toUCS2( bv+cl, d);
						} else {
							d = "";
						}
						comment.pwszDescription = (LPWSTR)(LPCWSTR)d;
						// get language
						if ( bvl - cl - dl > 0)
							memcpy( &comment.bLang, bv+cl+dl, sizeof(comment.bLang));

						// set the comment
						QCDCallbacks.toPlayer.SetTagData( tagHandle, wn, QTD_TYPE_BINARY, (BYTE *)&comment, sizeof(QTD_STRUCT_COMMENT), &index);
					} else if ( wn == QCDTag_Lyrics) { // for lyrics tag-group
						DWORD ll = 0, dl = 0;
						CStringW l, d;
						QTD_STRUCT_LYRICS lyrics; ZeroMemory( &lyrics, sizeof(QTD_STRUCT_LYRICS));

						// get lyrics
						ll = lstrlenA( (LPCSTR)bv) + 1;
						//l = new WCHAR[ll]; ZeroMemory( l, sizeof(WCHAR)*ll);
						UTF8toUCS2( bv, l);
						lyrics.pwszLyrics = (LPWSTR)(LPCWSTR)l;
						// get description
						if ( bvl - ll > 0) {
							dl = lstrlenA( (LPCSTR)(bv+ll)) + 1;
							UTF8toUCS2( bv+ll, d);
						} else {
							d = "";
						}
						lyrics.pwszDescription = (LPWSTR)(LPCWSTR)d;
						// get language
						if ( bvl - ll - dl > 0)
							memcpy( &lyrics.bLang, bv+ll+dl, sizeof(lyrics.bLang));

						// set the lyrics
						QCDCallbacks.toPlayer.SetTagData( tagHandle, wn, QTD_TYPE_BINARY, (BYTE *)&lyrics, sizeof(QTD_STRUCT_LYRICS), &index);
					} else if ( wn == QCDTag_GracenoteFileId || wn == QCDTag_GracenoteExtData) { // for CDDB id/data binary-data
						QCDCallbacks.toPlayer.SetTagData( tagHandle, wn, QTD_TYPE_BINARY, bv, bvl - 1, &index);
					} else {
						CStringW wv;

						UTF8toUCS2( (LPSTR)bv, wv);

						QCDCallbacks.toPlayer.SetTagData( tagHandle, wn, QTD_TYPE_STRINGUNICODE, (BYTE *)(LPCWSTR)wv, sizeof(WCHAR)*(wv.GetLength()+1), &index);
					}

					if ( bv) delete [] bv;

					++index;
				}

				if ( vcdata) delete vcdata;
			}

			break;
		case FLAC__METADATA_TYPE_PICTURE: // for artwork
			{
				// prepare and initialize artwork metadata instance
				FLAC::Metadata::Picture * picdata = dynamic_cast< FLAC::Metadata::Picture * >(iterator.get_block());
				if ( !picdata)
					return FALSE;

				// start reading & processing artwork metadata
				QTD_STRUCT_ARTWORK aw;

				CStringW mtstr;
				UTF8toUCS2( picdata->get_mime_type(), mtstr);
				aw.pszMimeType = (LPWSTR)(LPCWSTR)mtstr;

				aw.bPictureType = picdata->get_type();

				CStringW desstr;
				UTF8toUCS2( picdata->get_description(), desstr);
				aw.pwszDescription = (LPWSTR)(LPCWSTR)desstr;

				aw.dwDataLen = picdata->get_data_length();

				aw.pbData = (LPBYTE)picdata->get_data();

				QCDCallbacks.toPlayer.SetTagData( tagHandle, QCDTag_Artwork, QTD_TYPE_BINARY, (BYTE *)&aw, sizeof(QTD_STRUCT_ARTWORK), &index);

				++index;

				if ( picdata) delete picdata;
			}

		//	break;
		//case FLAC__METADATA_TYPE_APPLICATION:
		//	{
		//		FLAC::Metadata::Application * appdata = dynamic_cast< FLAC::Metadata::Picture * >(iterator.get_block());
		//		if ( !appdata)
		//			return FALSE;

		//		CHAR buf[5];
		//		memcpy( buf, appdata->get_id(), 4);
		//		buf[4] = '\0';
		//		CStringW tmp;

		//	}

			break;
		default:
			break;
		}
	} while ( iterator.next());

	// return true for successful read, false for failure
	return TRUE;
}

//-----------------------------------------------------------------------------

BOOL QMPTags::WriteToFile(LPCWSTR filename, void* tagHandle, int flags)
{
	FILE * fp;
	FLAC::Metadata::Chain chain;
	FLAC::Metadata::Iterator iterator;
	FLAC::Metadata::VorbisComment * vcdata;
	FLAC::Metadata::Picture * picdata;
	DWORD wnl, bvl;
	QCD_TAGDATA_TYPE type;
	int index;
	bool found;
	bool ret;

	// open the file and read all of its metadata
	fp = _wfopen( filename, L"r+b");
	if ( !fp) return FALSE;
	if ( !chain.read( fp, iocb, false)) {
		fclose( fp);
		return FALSE;
	}

	// update the metadata by enumerate all metadata with iterator
	if ( !iterator.is_valid()) {
		fclose( fp);
		return FALSE;
	}

	// initialize the iterator
	iterator.init( chain);

	// strip all tags
	if ( !_strip_all_tags( iterator)) {
		fclose( fp);
		return FALSE;
	}

	/************************************************************************/
	/* Append Vorbis Comment                                                */
	/************************************************************************/

	// find the valid vorbis comment block
	// or create a new one and insert it
	iterator.init( chain); // reset
	found = false;
	do {
		if ( FLAC__METADATA_TYPE_VORBIS_COMMENT == iterator.get_block_type()) {
			found = true;
			break;
		}
	} while ( iterator.next());
	if ( !found) { // create a new one
		vcdata = new FLAC::Metadata::VorbisComment;
		if ( !vcdata) {
			fclose( fp);
			return FALSE;
		}

		// pass the ownership to the chain
		vcdata->set_vendor_string( (FLAC__byte*)("QPlug-ins' FLAC Tagger"));
		iterator.insert_block_after( vcdata);
	}

	// get the block
	vcdata = dynamic_cast< FLAC::Metadata::VorbisComment * >(iterator.get_block());
	if ( !vcdata || FLAC__METADATA_TYPE_VORBIS_COMMENT != vcdata->get_type()) {
		if ( vcdata) delete vcdata;
		fclose( fp);
		return FALSE;
	}

	// append the tags one by one
	index = 0;
	wnl = bvl = 0;
	while ( QCDCallbacks.toPlayer.GetTagDataByIndex( tagHandle, index++, NULL, &wnl, &type, NULL, &bvl)) {
		// skip the ZERO length tag field
		if ( wnl <= 0 || bvl <= 0) continue;
		// get the information from the tag instance.
		++wnl;
		CStringW wn;
		LPBYTE bv = new BYTE[bvl]; ZeroMemory( bv, sizeof(BYTE)*bvl);
		// get the tag data.
		QCDCallbacks.toPlayer.GetTagDataByIndex( tagHandle, index-1, wn.GetBuffer( wnl), &wnl, &type, bv, &bvl);
		wn.ReleaseBuffer();
		wn.MakeUpper(); // always uppercase

		// the unique field for vorbis comment in UTF8 format
		CStringA name;
		LPSTR field, value;
		DWORD fieldlen, namelen, valuelen;

		// a vorbis comment's field name contains no embedded Nulls, 
		// so we convert it to UTF8 string directly
		UCS2toUTF8( wn, name);
		namelen = lstrlenA( name);

		if ( wn == QCDTag_Comment && type == QTD_TYPE_BINARY) {
			// the comment field is wrapped in the form "comment\0description\0lang"
			CStringA tmp;
			QTD_STRUCT_COMMENT * comment = (QTD_STRUCT_COMMENT *)bv;
			valuelen = ( lstrlenW( comment->pwszComment) + 1/*\0*/ + lstrlenW( comment->pwszDescription) + 1/*\0*/ + sizeof(comment->bLang)) * 3;
			value = new CHAR[valuelen]; ZeroMemory( value, sizeof(CHAR)*valuelen);
			// process comment
			UCS2toUTF8( comment->pwszComment, tmp);
			lstrcpyA( value, tmp);
			// process description next
			valuelen = lstrlenA( value);
			value[valuelen] = '\0';
			UCS2toUTF8( comment->pwszDescription, tmp);
			lstrcpyA( &value[valuelen+1], tmp);
			// process language finally
			valuelen += (1/*\0*/ + lstrlenA( &value[valuelen+1]));
			value[valuelen] = '\0';
			memcpy( &value[valuelen+1], comment->bLang, sizeof(comment->bLang));
			valuelen += (1/*\0*/ + sizeof(comment->bLang));
		} else if ( wn == QCDTag_Lyrics && type == QTD_TYPE_BINARY) {
			CStringA tmp;
			// the lyrics field is wrapped in the form "content\0description\0lang"
			QTD_STRUCT_LYRICS * lyrics = (QTD_STRUCT_LYRICS *)bv;
			valuelen = ( lstrlenW( lyrics->pwszLyrics) + 1/*\0*/ + lstrlenW( lyrics->pwszDescription) + 1/*\0*/ + sizeof(lyrics->bLang)) * 3;
			value = new CHAR[valuelen]; ZeroMemory( value, sizeof(CHAR)*valuelen);
			// process lyrics
			UCS2toUTF8( lyrics->pwszLyrics, tmp);
			lstrcpyA( value, tmp);
			// process description next
			valuelen = lstrlenA( value);
			value[valuelen] = '\0';
			UCS2toUTF8( lyrics->pwszDescription, tmp);
			lstrcpyA( &value[valuelen+1], tmp);
			// process language finally
			valuelen += (1/*\0*/ + lstrlenA( &value[valuelen+1]));
			value[valuelen] = '\0';
			memcpy( &value[valuelen+1], lyrics->bLang, sizeof(lyrics->bLang));
			valuelen += (1/*\0*/ + sizeof(lyrics->bLang));
		} else if ( type == QTD_TYPE_BINARY && (wn == QCDTag_GracenoteFileId || wn == QCDTag_GracenoteExtData)) {
			valuelen = bvl;
			value = new CHAR[valuelen]; ZeroMemory( value, sizeof(CHAR)*valuelen);
			memcpy( value, bv, valuelen);
		} else if ( type == QTD_TYPE_STRINGUNICODE) {
			// general field is wrapped in the form "content"
			CStringA tmp;
			UCS2toUTF8( (LPWSTR)bv, tmp);

			valuelen = tmp.GetLength() + 1;
			value = new CHAR[valuelen];
			lstrcpyA( value, tmp);
			valuelen = lstrlenA( value);
		} else if ( type == QTD_TYPE_LONG) {
			; // not support yet
		} else {
			// skip unsupported tag type
			// destroy all
			if ( bv) delete [] bv;
			continue;
		}

		// combine into one vorbis comment tag field
		fieldlen = namelen + 1/*=*/ + valuelen;
		field = new CHAR[fieldlen];
		lstrcpyA( field, name);
		lstrcatA( field, "=");
		memcpy( &field[namelen+1], value, valuelen);

		// append it to the empty vorbis comment block
		bool ok = vcdata->append_comment( FLAC::Metadata::VorbisComment::Entry( field, fieldlen));

		// destroy all
		if ( bv) delete [] bv;

		if ( field) delete [] field;
		if ( value) delete [] value;

		if ( !ok) {
			delete vcdata;
			fclose( fp);
			return FALSE;
		}
	}

	// clear data
	if ( vcdata) delete vcdata;

	/************************************************************************/
	/* Append Artworks                                                      */
	/************************************************************************/
	index = 0;
	bvl = 0;
	while ( QCDCallbacks.toPlayer.GetTagDataByName( tagHandle, QCDTag_Artwork, &type, NULL, &bvl, &index) && type == QTD_TYPE_BINARY) {
		// skip the ZERO length data tag
		if ( bvl > 0) {
			LPBYTE bv = new BYTE[bvl]; ZeroMemory( bv, bvl);
			QCDCallbacks.toPlayer.GetTagDataByName( tagHandle, QCDTag_Artwork, &type, bv, &bvl, &index);

			QTD_STRUCT_ARTWORK * aw = (QTD_STRUCT_ARTWORK *)bv;

			// create a new picture block
			iterator.insert_block_after( new FLAC::Metadata::Picture);
			picdata = dynamic_cast< FLAC::Metadata::Picture * >(iterator.get_block());

			picdata->set_type( (::FLAC__StreamMetadata_Picture_Type)aw->bPictureType);

			CStringA m;
			UCS2toUTF8( aw->pszMimeType, m);
			picdata->set_mime_type( m);

			CStringA d;
			UCS2toUTF8( aw->pwszDescription, d);
			picdata->set_description( (const FLAC__byte *)(LPCSTR)d);

			picdata->set_data( aw->pbData, aw->dwDataLen);

			// use default value from the picture
			picdata->set_width(0);
			picdata->set_height(0);
			picdata->set_depth(0);
			picdata->set_colors(0);

			// clear all
			if ( bv) delete [] bv;
			if ( picdata) delete picdata;
		}

		// next picture block
		++index;
	}

	/************************************************************************/
	/* Write tags into the file                                             */
	/************************************************************************/
	// sort padding
	chain.sort_padding();

	// write
	if ( chain.check_if_tempfile_needed( true)) {
		CStringW tmpfn(filename);
		tmpfn += L".meta";
		FILE * tmpfp = _wfopen( tmpfn, L"wb");
		if ( tmpfp) {
			ret = chain.write( true, fp, iocb, tmpfp, iocb);
			fclose( tmpfp);
		} else {
			ret = false;
		}
		fclose( fp);
		// move file
		if ( ret) ret = MoveFileExW( tmpfn, filename, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
	} else {
		ret = chain.write( true, fp, iocb);
		fclose( fp);
	}

	// return true for successful write, false for failure
	return ret;
}

//-----------------------------------------------------------------------------

BOOL QMPTags::StripFromFile(LPCWSTR filename, void* tagHandle, int flags)
{
	FILE * fp;
	FLAC::Metadata::Chain chain;
	FLAC::Metadata::Iterator iterator;
	bool ret;

	// open the file and read all of its metadata
	fp = _wfopen( filename, L"r+b");
	if ( !fp) return FALSE;
	if ( !chain.read( fp, iocb, false)) {
		fclose( fp);
		return FALSE;
	}

	// update the metadata by enumerate all metadata with iterator
	if ( !iterator.is_valid()) {
		fclose( fp);
		return FALSE;
	}

	iterator.init( chain);

	// strip all tags
	if ( !_strip_all_tags( iterator)) {
		fclose( fp);
		return FALSE;
	}

	// sort padding
	chain.sort_padding();

	// write the chain with paddings.
	// NO temporary files needed
	ret = chain.write( true, fp, iocb);

	// close file
	fclose( fp);

	return ret;
}

//-----------------------------------------------------------------------------

void QMPTags::About(int flags)
{
	CAboutDlgTags dlg;
	dlg.DoModal( (HWND)QCDCallbacks.Service( opGetPropertiesWnd, NULL, 0, 0));
}

//-----------------------------------------------------------------------------

void QMPTags::Configure(int flags)
{
	//
	// TODO : Show "configure" dialog.
	//
}

//-----------------------------------------------------------------------------

bool _strip_all_tags(FLAC::Metadata::Iterator & iterator)
{
	int index = 0;
	bool skip = false;
	do {
		switch ( iterator.get_block_type())
		{
		case FLAC__METADATA_TYPE_VORBIS_COMMENT:
			{
				// only leave the first vorbis comment
				if ( !skip) {
					FLAC::Metadata::Prototype * block = iterator.get_block();
					if ( !block) {
						return false;
					}

					// remove all tags
					if ( !::FLAC__metadata_object_vorbiscomment_resize_comments( 
						const_cast< FLAC__StreamMetadata * >((const FLAC__StreamMetadata *)(*block)), 
						0))
					{
						return false;
					}

					if ( block) delete block;

					skip = true;
					break;
				}
			}
			// fall through for remain vorbis comments
		case FLAC__METADATA_TYPE_PICTURE:
			{
				// delete artwork and replace with padding
				if ( !iterator.delete_block( true)) {
					return false;
				}
			}

			break;
		default:
			break;
		}
	} while ( iterator.next());

	return true;
}

