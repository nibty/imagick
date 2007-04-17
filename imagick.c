/*
   +----------------------------------------------------------------------+
   | PHP Version 5 / Imagick	                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 2006-2007 Mikko Koppanen, Scott MacVicar               |
   | Imagemagick (c) ImageMagick Studio LLC                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Mikko Kopppanen <mkoppanen@php.net>                          |
   |         Scott MacVicar <scottmac@php.net>                            |
   +----------------------------------------------------------------------+
*/

#include "php_imagick.h"

/* Class names */
#define PHP_IMAGICK_SC_NAME "Imagick"
#define PHP_IMAGICK_EXCEPTION_SC_NAME "ImagickException"

#define PHP_IMAGICKDRAW_SC_NAME "ImagickDraw"
#define PHP_IMAGICKDRAW_EXCEPTION_SC_NAME "ImagickDrawException"

#define PHP_IMAGICKPIXEL_SC_NAME "ImagickPixel"
#define PHP_IMAGICKPIXEL_EXCEPTION_SC_NAME "ImagickPixelException"

#define PHP_IMAGICKPIXELITERATOR_SC_NAME "ImagickPixelIterator"
#define PHP_IMAGICKPIXELITERATOR_EXCEPTION_SC_NAME "ImagickPixelIteratorException"


/* Handlers */
static zend_object_handlers imagick_object_handlers;
static zend_object_handlers imagickdraw_object_handlers;
static zend_object_handlers imagickpixeliterator_object_handlers;
static zend_object_handlers imagickpixel_object_handlers;

/* Class entries */
zend_class_entry *php_imagick_sc_entry;
zend_class_entry *php_imagick_exception_class_entry;
zend_class_entry *php_imagickdraw_sc_entry;
zend_class_entry *php_imagickdraw_exception_class_entry;
zend_class_entry *php_imagickpixeliterator_sc_entry;
zend_class_entry *php_imagickpixeliterator_exception_class_entry;
zend_class_entry *php_imagickpixel_sc_entry;
zend_class_entry *php_imagickpixel_exception_class_entry;

/* Borrowed from zip extension */
#define IMAGICK_REGISTER_CONST_LONG(const_name, value)\
	zend_declare_class_constant_long(php_imagick_sc_entry, const_name, sizeof(const_name)-1, (long)value TSRMLS_CC);

#define IMAGICK_CHECK_NOT_EMPTY( magick_wand, type, code )\
	if( getImageCount( magick_wand TSRMLS_CC) == 0 )\
	{ throwExceptionWithMessage( (long)type, "Can not process empty wand.", (long)code TSRMLS_CC); RETURN_FALSE; }

#define IMAGICK_INITIALIZE_ZERO_ARGS( object, wandType, intern )\
	if ( ZEND_NUM_ARGS() != 0 ) { ZEND_WRONG_PARAM_COUNT(); }\
	object = getThis();\
	intern = (wandType)zend_object_store_get_object(object TSRMLS_CC);

#define IMAGICKPIXEL_INITIALIZE_SET_COLOR( object, internp, color )\
	if ( ZEND_NUM_ARGS() != 1 ) { ZEND_WRONG_PARAM_COUNT(); }\
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &color ) == FAILURE ) { return; }\
	object = getThis();\
	internp = (php_imagickpixel_object *)zend_object_store_get_object(object TSRMLS_CC);

#define IMAGICK_FREE_MEMORY( type, value )\
	if ( value != (type) NULL ) { value = (type) MagickRelinquishMemory( value ); value = (type)NULL; }

#define IMAGICK_SAFE_MODE_CHECK( message, fileName )\
	if( PG(safe_mode) ) {\
		if ( php_check_open_basedir_ex( fileName, 0 TSRMLS_CC ) || php_checkuid_ex( fileName, NULL, CHECKUID_CHECK_FILE_AND_DIR, CHECKUID_NO_ERRORS ) ){\
		zend_throw_exception_ex( php_imagick_exception_class_entry, 1 TSRMLS_CC,\
								 message, fileName); efree( fileName ); RETURN_FALSE; } }

#define IMAGICK_HAS_FORMAT( buffer, magick_wand )\
	buffer = MagickGetImageFormat( magick_wand );\
	if( buffer == (char *)NULL || *buffer == '\0' ) {\
	IMAGICK_FREE_MEMORY( char *, buffer ); throwExceptionWithMessage( 1, "Image has no format.", 1 TSRMLS_CC ); RETURN_FALSE;}\
	else { IMAGICK_FREE_MEMORY( char *, buffer ); }

/* Forward declarations (Imagick) */
PHP_METHOD(imagick, __construct);
PHP_METHOD(imagick, readimage);
PHP_METHOD(imagick, pingimage);
PHP_METHOD(imagick, readimageblob);
PHP_METHOD(imagick, pingimageblob);
PHP_METHOD(imagick, readimagefile);
PHP_METHOD(imagick, pingimagefile);
PHP_METHOD(imagick, destroy);
PHP_METHOD(imagick, clear);
PHP_METHOD(imagick, clone);
PHP_METHOD(imagick, removeimage);
PHP_METHOD(imagick, writeimage);
PHP_METHOD(imagick, writeimages);
PHP_METHOD(imagick, resetiterator);
PHP_METHOD(imagick, setfirstiterator);
PHP_METHOD(imagick, previousimage);
PHP_METHOD(imagick, nextimage);
PHP_METHOD(imagick, haspreviousimage);
PHP_METHOD(imagick, hasnextimage);
PHP_METHOD(imagick, setimageindex);
PHP_METHOD(imagick, getimageindex);
PHP_METHOD(imagick, setimageformat);
PHP_METHOD(imagick, setimagefilename);
PHP_METHOD(imagick, getimagefilename);
PHP_METHOD(imagick, getimageformat);
PHP_METHOD(imagick, getimagesize);
PHP_METHOD(imagick, getimagegeometry);
PHP_METHOD(imagick, getimagecolors);
PHP_METHOD(imagick, scaleimage);
PHP_METHOD(imagick, blurimage);
PHP_METHOD(imagick, blurimagechannel);
PHP_METHOD(imagick, thumbnailimage);
PHP_METHOD(imagick, commentimage);
PHP_METHOD(imagick, cropimage);
PHP_METHOD(imagick, labelimage);
PHP_METHOD(imagick, valid);
PHP_METHOD(imagick, current);
PHP_METHOD(imagick, drawimage);
PHP_METHOD(imagick, getimageblob);
PHP_METHOD(imagick, setimagecompressionquality);
PHP_METHOD(imagick, annotateimage);
PHP_METHOD(imagick, compositeimage);
PHP_METHOD(imagick, compositeimagechannel);
PHP_METHOD(imagick, modulateimage);
PHP_METHOD(imagick, montageimage);
PHP_METHOD(imagick, identifyimage);
PHP_METHOD(imagick, thresholdimage);
PHP_METHOD(imagick, thresholdimagechannel);
PHP_METHOD(imagick, randomthresholdimage);
PHP_METHOD(imagick, randomthresholdimagechannel);
PHP_METHOD(imagick, adaptivethresholdimage);
PHP_METHOD(imagick, blackthresholdimage);
PHP_METHOD(imagick, whitethresholdimage);
PHP_METHOD(imagick, appendimages);
PHP_METHOD(imagick, charcoalimage);
PHP_METHOD(imagick, normalizeimage);
PHP_METHOD(imagick, normalizeimagechannel);
PHP_METHOD(imagick, oilpaintimage);
PHP_METHOD(imagick, posterizeimage);
PHP_METHOD(imagick, radialblurimage);
PHP_METHOD(imagick, radialblurimagechannel);
PHP_METHOD(imagick, raiseimage);
PHP_METHOD(imagick, resampleimage);
PHP_METHOD(imagick, resizeimage);
PHP_METHOD(imagick, rollimage);
PHP_METHOD(imagick, rotateimage);
PHP_METHOD(imagick, sampleimage);
PHP_METHOD(imagick, solarizeimage);
PHP_METHOD(imagick, shadeimage);
PHP_METHOD(imagick, shadowimage);
PHP_METHOD(imagick, setimageattribute);
PHP_METHOD(imagick, setimagebackgroundcolor);
PHP_METHOD(imagick, setimagecompose);
PHP_METHOD(imagick, setimagecompression);
PHP_METHOD(imagick, setimagedelay);
PHP_METHOD(imagick, setimagedepth);
PHP_METHOD(imagick, setimagegamma);
PHP_METHOD(imagick, setimageiterations);
PHP_METHOD(imagick, setimagematte);
PHP_METHOD(imagick, setimagemattecolor);
PHP_METHOD(imagick, setimagepage);
PHP_METHOD(imagick, setimageresolution);
PHP_METHOD(imagick, setimagescene);
PHP_METHOD(imagick, setimagetickspersecond);
PHP_METHOD(imagick, setimagetype);
PHP_METHOD(imagick, setimageunits);
PHP_METHOD(imagick, sharpenimage);
PHP_METHOD(imagick, sharpenimagechannel);
PHP_METHOD(imagick, shaveimage);
PHP_METHOD(imagick, shearimage);
PHP_METHOD(imagick, sketchimage);
PHP_METHOD(imagick, spliceimage);
PHP_METHOD(imagick, spreadimage);
PHP_METHOD(imagick, swirlimage);
PHP_METHOD(imagick, stripimage);
PHP_METHOD(imagick, transposeimage);
PHP_METHOD(imagick, transverseimage);
PHP_METHOD(imagick, trimimage);
PHP_METHOD(imagick, waveimage);
PHP_METHOD(imagick, vignetteimage);
PHP_METHOD(imagick, getpixeliterator);
PHP_METHOD(imagick, getpixelregioniterator);
PHP_METHOD(imagick, queryformats);
PHP_METHOD(imagick, steganoimage);
PHP_METHOD(imagick, motionblurimage);
PHP_METHOD(imagick, mosaicimages);
PHP_METHOD(imagick, morphimages);
PHP_METHOD(imagick, minifyimage);
PHP_METHOD(imagick, addnoiseimage);
PHP_METHOD(imagick, addnoiseimagechannel);
PHP_METHOD(imagick, adaptiveresizeimage);
PHP_METHOD(imagick, adaptivesharpenimage);
PHP_METHOD(imagick, adaptivesharpenimagechannel);
PHP_METHOD(imagick, affinetransformimage);
PHP_METHOD(imagick, averageimages);
PHP_METHOD(imagick, borderimage);
PHP_METHOD(imagick, chopimage);
PHP_METHOD(imagick, clipimage);
PHP_METHOD(imagick, clippathimage);
PHP_METHOD(imagick, coalesceimages);
PHP_METHOD(imagick, colorfloodfillimage);
PHP_METHOD(imagick, colorizeimage);
PHP_METHOD(imagick, compareimagechannels);
PHP_METHOD(imagick, compareimagelayers);
PHP_METHOD(imagick, compareimages);
PHP_METHOD(imagick, contrastimage);
PHP_METHOD(imagick, combineimages);
PHP_METHOD(imagick, contraststretchimage);
PHP_METHOD(imagick, contraststretchimagechannel);
PHP_METHOD(imagick, convolveimage);
PHP_METHOD(imagick, convolveimagechannel);
PHP_METHOD(imagick, cyclecolormapimage);
PHP_METHOD(imagick, deconstructimages);
PHP_METHOD(imagick, despeckleimage);
PHP_METHOD(imagick, edgeimage);
PHP_METHOD(imagick, embossimage);
PHP_METHOD(imagick, enhanceimage);
PHP_METHOD(imagick, equalizeimage);
PHP_METHOD(imagick, evaluateimage);
PHP_METHOD(imagick, evaluateimagechannel);
PHP_METHOD(imagick, adaptiveblurimage);
PHP_METHOD(imagick, adaptiveblurimagechannel);
PHP_METHOD(imagick, flattenimages);
PHP_METHOD(imagick, flipimage);
PHP_METHOD(imagick, flopimage);
PHP_METHOD(imagick, frameimage);
PHP_METHOD(imagick, fximage);
PHP_METHOD(imagick, fximagechannel);
PHP_METHOD(imagick, gammaimage);
PHP_METHOD(imagick, gammaimagechannel);
PHP_METHOD(imagick, gaussianblurimage);
PHP_METHOD(imagick, gaussianblurimagechannel);
PHP_METHOD(imagick, getimagebackgroundcolor);
PHP_METHOD(imagick, getimageblueprimary);
PHP_METHOD(imagick, getimagebordercolor);
PHP_METHOD(imagick, getimagechanneldepth);
PHP_METHOD(imagick, getimagechanneldistortion);
PHP_METHOD(imagick, getimagechannelextrema);
PHP_METHOD(imagick, getimagechannelmean);
PHP_METHOD(imagick, getimagechannelstatistics);
PHP_METHOD(imagick, getimagecolormapcolor);
PHP_METHOD(imagick, getimagecolorspace);
PHP_METHOD(imagick, getimagecompose);
PHP_METHOD(imagick, getimagedelay);
PHP_METHOD(imagick, getimagedepth);
PHP_METHOD(imagick, getimagedistortion);
PHP_METHOD(imagick, getimageextrema);
PHP_METHOD(imagick, getimagedispose);
PHP_METHOD(imagick, getimagegamma);
PHP_METHOD(imagick, getimagegreenprimary);
PHP_METHOD(imagick, getimageheight);
PHP_METHOD(imagick, getimagehistogram);
PHP_METHOD(imagick, getimageinterlacescheme);
PHP_METHOD(imagick, getimageinterpolatemethod);
PHP_METHOD(imagick, getimageiterations);
PHP_METHOD(imagick, getimagematte);
PHP_METHOD(imagick, getimagemattecolor);
PHP_METHOD(imagick, getimagepage);
PHP_METHOD(imagick, getimagepixelcolor);
PHP_METHOD(imagick, getimageprofile);
PHP_METHOD(imagick, getimageredprimary);
PHP_METHOD(imagick, getimagerenderingintent);
PHP_METHOD(imagick, getimageresolution);
PHP_METHOD(imagick, getimagescene);
PHP_METHOD(imagick, getimagesignature);
PHP_METHOD(imagick, getimagetickspersecond);
PHP_METHOD(imagick, getimagetype);
PHP_METHOD(imagick, getimageunits);
PHP_METHOD(imagick, getimagevirtualpixelmethod);
PHP_METHOD(imagick, getimagewhitepoint);
PHP_METHOD(imagick, getimagewidth);
PHP_METHOD(imagick, getnumberimages);
PHP_METHOD(imagick, getimagetotalinkdensity);
PHP_METHOD(imagick, getimageregion);
PHP_METHOD(imagick, implodeimage);
PHP_METHOD(imagick, levelimage);
PHP_METHOD(imagick, levelimagechannel);
PHP_METHOD(imagick, linearstretchimage);
PHP_METHOD(imagick, magnifyimage);
PHP_METHOD(imagick, mapimage);
PHP_METHOD(imagick, mattefloodfillimage);
PHP_METHOD(imagick, medianfilterimage);
PHP_METHOD(imagick, negateimage);
PHP_METHOD(imagick, negateimagechannel);
PHP_METHOD(imagick, paintopaqueimage);
PHP_METHOD(imagick, paintopaqueimagechannel);
PHP_METHOD(imagick, optimizeimagelayers);
PHP_METHOD(imagick, painttransparentimage);
PHP_METHOD(imagick, previewimages);
PHP_METHOD(imagick, profileimage);
PHP_METHOD(imagick, quantizeimage);
PHP_METHOD(imagick, quantizeimages);
PHP_METHOD(imagick, reducenoiseimage);
PHP_METHOD(imagick, removeimageprofile);
PHP_METHOD(imagick, separateimagechannel);
PHP_METHOD(imagick, sepiatoneimage);
PHP_METHOD(imagick, setimagebias);
PHP_METHOD(imagick, setimageblueprimary);
PHP_METHOD(imagick, setimagebordercolor);
PHP_METHOD(imagick, setimagechanneldepth);
PHP_METHOD(imagick, setimagecolormapcolor);
PHP_METHOD(imagick, setimagecolorspace);
PHP_METHOD(imagick, setimagedispose);
PHP_METHOD(imagick, setimageextent);
PHP_METHOD(imagick, setimagegreenprimary);
PHP_METHOD(imagick, setimageinterlacescheme);
PHP_METHOD(imagick, setimageinterpolatemethod);
PHP_METHOD(imagick, setimageprofile);
PHP_METHOD(imagick, setimageredprimary);
PHP_METHOD(imagick, setimagerenderingintent);
PHP_METHOD(imagick, setimagevirtualpixelmethod);
PHP_METHOD(imagick, setimagewhitepoint);
PHP_METHOD(imagick, sigmoidalcontrastimage);
PHP_METHOD(imagick, sigmoidalcontrastimagechannel);
PHP_METHOD(imagick, stereoimage);
PHP_METHOD(imagick, textureimage);
PHP_METHOD(imagick, tintimage);
PHP_METHOD(imagick, uniqueimagecolors);
PHP_METHOD(imagick, unsharpmaskimage);
PHP_METHOD(imagick, unsharpmaskimagechannel);
PHP_METHOD(imagick, getimage);
PHP_METHOD(imagick, addimage);
PHP_METHOD(imagick, newimage);
PHP_METHOD(imagick, getcompression);
PHP_METHOD(imagick, getcompressionquality);
PHP_METHOD(imagick, getcopyright);
PHP_METHOD(imagick, getfilename);
PHP_METHOD(imagick, getformat);
PHP_METHOD(imagick, gethomeurl);
PHP_METHOD(imagick, getinterlacescheme);
PHP_METHOD(imagick, getoption);
PHP_METHOD(imagick, getpackagename);
PHP_METHOD(imagick, getpage);
PHP_METHOD(imagick, getquantumdepth);
PHP_METHOD(imagick, getquantumrange);
PHP_METHOD(imagick, getreleasedate);
PHP_METHOD(imagick, getresource);
PHP_METHOD(imagick, getresourcelimit);
PHP_METHOD(imagick, getsamplingfactors);
PHP_METHOD(imagick, getsize);
PHP_METHOD(imagick, getsizeoffset);
PHP_METHOD(imagick, getversion);
PHP_METHOD(imagick, setbackgroundcolor);
PHP_METHOD(imagick, setcompression);
PHP_METHOD(imagick, setcompressionquality);
PHP_METHOD(imagick, setfilename);
PHP_METHOD(imagick, setformat);
PHP_METHOD(imagick, setinterlacescheme);
PHP_METHOD(imagick, setoption);
PHP_METHOD(imagick, setpage);
PHP_METHOD(imagick, setresourcelimit);
PHP_METHOD(imagick, setresolution);
PHP_METHOD(imagick, setsamplingfactors);
PHP_METHOD(imagick, setsize);
PHP_METHOD(imagick, setsizeoffset);
PHP_METHOD(imagick, settype);


/* Forward declarations (ImagickDraw) */
PHP_METHOD(imagickdraw, __construct);
PHP_METHOD(imagickdraw, setfillcolor);
PHP_METHOD(imagickdraw, setfillalpha);
PHP_METHOD(imagickdraw, setstrokecolor);
PHP_METHOD(imagickdraw, setstrokealpha);
PHP_METHOD(imagickdraw, setstrokewidth);
PHP_METHOD(imagickdraw, setfont);
PHP_METHOD(imagickdraw, setfontfamily);
PHP_METHOD(imagickdraw, setfontsize);
PHP_METHOD(imagickdraw, setfontstyle);
PHP_METHOD(imagickdraw, setfontweight);
PHP_METHOD(imagickdraw, getfont);
PHP_METHOD(imagickdraw, getfontfamily);
PHP_METHOD(imagickdraw, getfontsize);
PHP_METHOD(imagickdraw, getfontstyle);
PHP_METHOD(imagickdraw, getfontweight);
PHP_METHOD(imagickdraw, clear);
PHP_METHOD(imagickdraw, destroy);
PHP_METHOD(imagickdraw, annotation);
PHP_METHOD(imagickdraw, settextantialias);
PHP_METHOD(imagickdraw, settextencoding);
PHP_METHOD(imagickdraw, rectangle);
PHP_METHOD(imagickdraw, roundrectangle);
PHP_METHOD(imagickdraw, ellipse);
PHP_METHOD(imagickdraw, circle);
PHP_METHOD(imagickdraw, skewx);
PHP_METHOD(imagickdraw, skewy);
PHP_METHOD(imagickdraw, translate);
PHP_METHOD(imagickdraw, line);
PHP_METHOD(imagickdraw, polygon);
PHP_METHOD(imagickdraw, arc);
PHP_METHOD(imagickdraw, matte);
PHP_METHOD(imagickdraw, point);
PHP_METHOD(imagickdraw, setfontstretch);
PHP_METHOD(imagickdraw, settextalignment);
PHP_METHOD(imagickdraw, settextdecoration);
PHP_METHOD(imagickdraw, settextundercolor);
PHP_METHOD(imagickdraw, setviewbox);
PHP_METHOD(imagickdraw, gettextdecoration);
PHP_METHOD(imagickdraw, gettextencoding);
PHP_METHOD(imagickdraw, clone);
PHP_METHOD(imagickdraw, affine);
PHP_METHOD(imagickdraw, bezier);
PHP_METHOD(imagickdraw, composite);
PHP_METHOD(imagickdraw, color);
PHP_METHOD(imagickdraw, comment);
PHP_METHOD(imagickdraw, getclippath);
PHP_METHOD(imagickdraw, getcliprule);
PHP_METHOD(imagickdraw, getclipunits);
PHP_METHOD(imagickdraw, getfillcolor);
PHP_METHOD(imagickdraw, getfillopacity);
PHP_METHOD(imagickdraw, getfillrule);
PHP_METHOD(imagickdraw, getgravity);
PHP_METHOD(imagickdraw, getstrokeantialias);
PHP_METHOD(imagickdraw, getstrokecolor);
PHP_METHOD(imagickdraw, getstrokedasharray);
PHP_METHOD(imagickdraw, getstrokedashoffset);
PHP_METHOD(imagickdraw, getstrokelinecap);
PHP_METHOD(imagickdraw, getstrokelinejoin);
PHP_METHOD(imagickdraw, getstrokemiterlimit);
PHP_METHOD(imagickdraw, getstrokeopacity);
PHP_METHOD(imagickdraw, getstrokewidth);
PHP_METHOD(imagickdraw, gettextalignment);
PHP_METHOD(imagickdraw, gettextantialias);
PHP_METHOD(imagickdraw, getvectorgraphics);
PHP_METHOD(imagickdraw, gettextundercolor);
PHP_METHOD(imagickdraw, pathclose);
PHP_METHOD(imagickdraw, pathcurvetoabsolute);
PHP_METHOD(imagickdraw, pathcurvetorelative);
PHP_METHOD(imagickdraw, pathcurvetoquadraticbezierabsolute);
PHP_METHOD(imagickdraw, pathcurvetoquadraticbezierrelative);
PHP_METHOD(imagickdraw, pathcurvetoquadraticbeziersmoothabsolute);
PHP_METHOD(imagickdraw, pathcurvetoquadraticbeziersmoothrelative);
PHP_METHOD(imagickdraw, pathcurvetosmoothabsolute);
PHP_METHOD(imagickdraw, pathcurvetosmoothrelative);
PHP_METHOD(imagickdraw, pathellipticarcabsolute);
PHP_METHOD(imagickdraw, pathellipticarcrelative);
PHP_METHOD(imagickdraw, pathfinish);
PHP_METHOD(imagickdraw, pathlinetoabsolute);
PHP_METHOD(imagickdraw, pathlinetorelative);
PHP_METHOD(imagickdraw, pathlinetohorizontalabsolute);
PHP_METHOD(imagickdraw, pathlinetohorizontalrelative);
PHP_METHOD(imagickdraw, pathlinetoverticalabsolute);
PHP_METHOD(imagickdraw, pathlinetoverticalrelative);
PHP_METHOD(imagickdraw, pathmovetoabsolute);
PHP_METHOD(imagickdraw, pathmovetorelative);
PHP_METHOD(imagickdraw, pathstart);
PHP_METHOD(imagickdraw, polyline);
PHP_METHOD(imagickdraw, popclippath);
PHP_METHOD(imagickdraw, popdefs);
PHP_METHOD(imagickdraw, poppattern);
PHP_METHOD(imagickdraw, pushclippath);
PHP_METHOD(imagickdraw, pushdefs);
PHP_METHOD(imagickdraw, pushpattern);
PHP_METHOD(imagickdraw, render);
PHP_METHOD(imagickdraw, resetvectorgraphics);
PHP_METHOD(imagickdraw, rotate);
PHP_METHOD(imagickdraw, scale);
PHP_METHOD(imagickdraw, setclippath);
PHP_METHOD(imagickdraw, setcliprule);
PHP_METHOD(imagickdraw, setclipunits);
PHP_METHOD(imagickdraw, setfillopacity);
PHP_METHOD(imagickdraw, setfillpatternurl);
PHP_METHOD(imagickdraw, setfillrule);
PHP_METHOD(imagickdraw, setgravity);
PHP_METHOD(imagickdraw, setstrokepatternurl);
PHP_METHOD(imagickdraw, setstrokeantialias);
PHP_METHOD(imagickdraw, setstrokedasharray);
PHP_METHOD(imagickdraw, setstrokedashoffset);
PHP_METHOD(imagickdraw, setstrokelinecap);
PHP_METHOD(imagickdraw, setstrokelinejoin);
PHP_METHOD(imagickdraw, setstrokemiterlimit);
PHP_METHOD(imagickdraw, setstrokeopacity);
PHP_METHOD(imagickdraw, setvectorgraphics);
PHP_METHOD(imagickdraw, popdrawingwand);
PHP_METHOD(imagickdraw, pushdrawingwand);

/* Imagick Pixel iterator */
PHP_METHOD(imagickpixeliterator, __construct);
PHP_METHOD(imagickpixeliterator, newpixeliterator);
PHP_METHOD(imagickpixeliterator, newpixelregioniterator);
PHP_METHOD(imagickpixeliterator, getiteratorrow);
PHP_METHOD(imagickpixeliterator, setiteratorrow);
PHP_METHOD(imagickpixeliterator, getpreviousiteratorrow);
PHP_METHOD(imagickpixeliterator, getcurrentiteratorrow);
PHP_METHOD(imagickpixeliterator, getnextiteratorrow);
PHP_METHOD(imagickpixeliterator, setiteratorfirstrow);
PHP_METHOD(imagickpixeliterator, setiteratorlastrow);
PHP_METHOD(imagickpixeliterator, resetiterator);
PHP_METHOD(imagickpixeliterator, synciterator);
PHP_METHOD(imagickpixeliterator, destroy);
PHP_METHOD(imagickpixeliterator, clear);

/* ImagickPixel */
PHP_METHOD(imagickpixel, __construct);
PHP_METHOD(imagickpixel, setcolor);
PHP_METHOD(imagickpixel, clear);
PHP_METHOD(imagickpixel, destroy);
PHP_METHOD(imagickpixel, setalpha);
PHP_METHOD(imagickpixel, getalpha);
PHP_METHOD(imagickpixel, issimilar);
PHP_METHOD(imagickpixel, getblack);
PHP_METHOD(imagickpixel, setblack);
PHP_METHOD(imagickpixel, getblue);
PHP_METHOD(imagickpixel, setblue);
PHP_METHOD(imagickpixel, getcyan);
PHP_METHOD(imagickpixel, setcyan);
PHP_METHOD(imagickpixel, getgreen);
PHP_METHOD(imagickpixel, setgreen);
PHP_METHOD(imagickpixel, getred);
PHP_METHOD(imagickpixel, setred);
PHP_METHOD(imagickpixel, getyellow);
PHP_METHOD(imagickpixel, setyellow);
PHP_METHOD(imagickpixel, getmagenta);
PHP_METHOD(imagickpixel, setmagenta);
PHP_METHOD(imagickpixel, getopacity);
PHP_METHOD(imagickpixel, setopacity);
PHP_METHOD(imagickpixel, getfuzz);
PHP_METHOD(imagickpixel, setfuzz);
PHP_METHOD(imagickpixel, gethsl);
PHP_METHOD(imagickpixel, sethsl);
PHP_METHOD(imagickpixel, getcolorasstring);
PHP_METHOD(imagickpixel, getcolorasnormalizedstring);
PHP_METHOD(imagickpixel, getcolorcount);

static function_entry php_imagick_functions[] =
{
	{ NULL, NULL, NULL }
};

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_zero_args, 0, 0, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setfillcolor_args, 0, 0, 1)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setfillalpha_args, 0, 0, 1)
		ZEND_ARG_INFO(0, alpha)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setstrokecolor_args, 0, 0, 1)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setstrokealpha_args, 0, 0, 1)
		ZEND_ARG_INFO(0, alpha)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setstrokewidth_args, 0, 0, 1)
		ZEND_ARG_INFO(0, width)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_circle_args, 0, 0, 4)
		ZEND_ARG_INFO(0, ox)
		ZEND_ARG_INFO(0, oy)
		ZEND_ARG_INFO(0, px)
		ZEND_ARG_INFO(0, py)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_annotation_args, 0, 0, 3)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
		ZEND_ARG_INFO(0, text)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_settextantialias_args, 0, 0, 1)
		ZEND_ARG_INFO(0, antialias)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_settextencoding_args, 0, 0, 1)
		ZEND_ARG_INFO(0, encoding)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setfont_args, 0, 0, 1)
		ZEND_ARG_INFO(0, font)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setfontfamily_args, 0, 0, 1)
		ZEND_ARG_INFO(0, fontfamily)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setfontsize_args, 0, 0, 1)
		ZEND_ARG_INFO(0, pointsize)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setfontstyle_args, 0, 0, 1)
		ZEND_ARG_INFO(0, STYLE)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setfontweight_args, 0, 0, 1)
		ZEND_ARG_INFO(0, weight)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_rectangle_args, 0, 0, 4)
		ZEND_ARG_INFO(0, x1)
		ZEND_ARG_INFO(0, y1)
		ZEND_ARG_INFO(0, x2)
		ZEND_ARG_INFO(0, y2)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_roundrectangle_args, 0, 0, 6)
		ZEND_ARG_INFO(0, x1)
		ZEND_ARG_INFO(0, y1)
		ZEND_ARG_INFO(0, x2)
		ZEND_ARG_INFO(0, y2)
		ZEND_ARG_INFO(0, rx)
		ZEND_ARG_INFO(0, ry)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_ellipse_args, 0, 0, 6)
		ZEND_ARG_INFO(0, ox)
		ZEND_ARG_INFO(0, oy)
		ZEND_ARG_INFO(0, px)
		ZEND_ARG_INFO(0, py)
		ZEND_ARG_INFO(0, start)
		ZEND_ARG_INFO(0, end)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_skewx_args, 0, 0, 1)
		ZEND_ARG_INFO(0, degrees)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_skewy_args, 0, 0, 1)
		ZEND_ARG_INFO(0, degrees)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_translate_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_line_args, 0, 0, 4)
		ZEND_ARG_INFO(0, sx)
		ZEND_ARG_INFO(0, sy)
		ZEND_ARG_INFO(0, ex)
		ZEND_ARG_INFO(0, ey)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_arc_args, 0, 0, 6)
		ZEND_ARG_INFO(0, sx)
		ZEND_ARG_INFO(0, sy)
		ZEND_ARG_INFO(0, ex)
		ZEND_ARG_INFO(0, ey)
		ZEND_ARG_INFO(0, sd)
		ZEND_ARG_INFO(0, ed)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_matte_args, 0, 0, 3)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
		ZEND_ARG_INFO(0, METHOD)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_polygon_args, 0, 0, 1)
		ZEND_ARG_INFO(0, coordinates)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_point_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setfontstretch_args, 0, 0, 1)
		ZEND_ARG_INFO(0, STRETCH)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setstrokeantialias_args, 0, 0, 1)
		ZEND_ARG_INFO(0, antialias)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setfontalignment_args, 0, 0, 1)
		ZEND_ARG_INFO(0, ALIGN)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_settextdecoration_args, 0, 0, 1)
		ZEND_ARG_INFO(0, DECORATION)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_settextundercolor_args, 0, 0, 1)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setviewbox_args, 0, 0, 4)
		ZEND_ARG_INFO(0, sx)
		ZEND_ARG_INFO(0, sy)
		ZEND_ARG_INFO(0, ex)
		ZEND_ARG_INFO(0, ey)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_affine_args, 0, 0, 1)
		ZEND_ARG_INFO(0, affineMatrix)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_bezier_args, 0, 0, 1)
		ZEND_ARG_INFO(0, coordinateArray)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_composite_args, 0, 0, 6)
		ZEND_ARG_INFO(0, COMPOSE)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
		ZEND_ARG_INFO(0, width)
		ZEND_ARG_INFO(0, height)
		ZEND_ARG_OBJ_INFO(0, Imagick, Imagick, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_color_args, 0, 0, 3)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
		ZEND_ARG_INFO(0, PAINTMETHOD)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_comment_args, 0, 0, 1)
		ZEND_ARG_INFO(0, comment)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathcurvetoabsolute_args, 0, 0, 6)
		ZEND_ARG_INFO(0, x1)
		ZEND_ARG_INFO(0, y1)
		ZEND_ARG_INFO(0, x2)
		ZEND_ARG_INFO(0, y2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathcurvetorelative_args, 0, 0, 6)
		ZEND_ARG_INFO(0, x1)
		ZEND_ARG_INFO(0, y1)
		ZEND_ARG_INFO(0, x2)
		ZEND_ARG_INFO(0, y2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathcurvetoquadraticbezierabsolute_args, 0, 0, 4)
		ZEND_ARG_INFO(0, x1)
		ZEND_ARG_INFO(0, y1)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathcurvetoquadraticbezierrelative_args, 0, 0, 4)
		ZEND_ARG_INFO(0, x1)
		ZEND_ARG_INFO(0, y1)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathcurvetoquadraticbeziersmoothabsolute_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathcurvetoquadraticbeziersmoothrelative_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathcurvetosmoothabsolute_args, 0, 0, 4)
		ZEND_ARG_INFO(0, x1)
		ZEND_ARG_INFO(0, y1)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathcurvetosmoothrelative_args, 0, 0, 4)
		ZEND_ARG_INFO(0, x1)
		ZEND_ARG_INFO(0, y1)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathellipticarcabsolute_args, 0, 0, 7)
		ZEND_ARG_INFO(0, rx)
		ZEND_ARG_INFO(0, ry)
		ZEND_ARG_INFO(0, xAxisRotation)
		ZEND_ARG_INFO(0, largeArc)
		ZEND_ARG_INFO(0, sweep)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathellipticarcrelative_args, 0, 0, 7)
		ZEND_ARG_INFO(0, rx)
		ZEND_ARG_INFO(0, ry)
		ZEND_ARG_INFO(0, xAxisRotation)
		ZEND_ARG_INFO(0, largeArc)
		ZEND_ARG_INFO(0, sweep)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathlinetoabsolute_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathlinetorelative_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathlinetohorizontalabsolute_args, 0, 0, 1)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathlinetohorizontalrelative_args, 0, 0, 1)
		ZEND_ARG_INFO(0, x)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathlinetoverticalabsolute_args, 0, 0, 1)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathlinetoverticalrelative_args, 0, 0, 1)
		ZEND_ARG_INFO(0, x)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathmovetoabsolute_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pathmovetorelative_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_polyline_args, 0, 0, 1)
		ZEND_ARG_INFO(0, coordinateArray)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pushclippath_args, 0, 0, 1)
		ZEND_ARG_INFO(0, clipMask)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_pushpattern_args, 0, 0, 5)
		ZEND_ARG_INFO(0, patternId)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
		ZEND_ARG_INFO(0, width)
		ZEND_ARG_INFO(0, height)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_rotate_args, 0, 0, 1)
		ZEND_ARG_INFO(0, degrees)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_scale_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setclippath_args, 0, 0, 1)
		ZEND_ARG_INFO(0, clipMask)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setcliprule_args, 0, 0, 1)
		ZEND_ARG_INFO(0, FILLRULE)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setclipunits_args, 0, 0, 1)
		ZEND_ARG_INFO(0, PATHUNITS)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setfillopacity_args, 0, 0, 1)
		ZEND_ARG_INFO(0, fillOpacity)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setfillpatternurl_args, 0, 0, 1)
		ZEND_ARG_INFO(0, url)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setfillrule_args, 0, 0, 1)
		ZEND_ARG_INFO(0, FILLRULE)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setgravity_args, 0, 0, 1)
		ZEND_ARG_INFO(0, GRAVITY)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setstrokepatternurl_args, 0, 0, 1)
		ZEND_ARG_INFO(0, url)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setstrokedashoffset_args, 0, 0, 1)
		ZEND_ARG_INFO(0, offset)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setstrokelinecap_args, 0, 0, 1)
		ZEND_ARG_INFO(0, LINECAP)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setstrokelinejoin_args, 0, 0, 1)
		ZEND_ARG_INFO(0, LINEJOIN)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setstrokemiterlimit_args, 0, 0, 1)
		ZEND_ARG_INFO(0, miterLimit)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setstrokeopacity_args, 0, 0, 1)
		ZEND_ARG_INFO(0, strokeOpacity)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setvectorgraphics_args, 0, 0, 1)
		ZEND_ARG_INFO(0, xml)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickdraw_setstrokedasharray_args, 0, 0, 1)
		ZEND_ARG_INFO(0, dashArray)
	ZEND_END_ARG_INFO()

static function_entry php_imagickdraw_class_methods[] =
{
	PHP_ME(imagickdraw, __construct, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setfillcolor, imagickdraw_setfillcolor_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setfillalpha, imagickdraw_setfillalpha_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setstrokecolor, imagickdraw_setstrokecolor_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setstrokealpha, imagickdraw_setstrokealpha_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setstrokewidth, imagickdraw_setstrokewidth_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, clear, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, circle, imagickdraw_circle_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, annotation, imagickdraw_annotation_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, settextantialias, imagickdraw_settextantialias_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, settextencoding, imagickdraw_settextencoding_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setfont, imagickdraw_setfont_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setfontfamily, imagickdraw_setfontfamily_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setfontsize, imagickdraw_setfontsize_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setfontstyle, imagickdraw_setfontstyle_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setfontweight, imagickdraw_setfontweight_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getfont, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getfontfamily, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getfontsize, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getfontstyle, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getfontweight, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, destroy, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, rectangle, imagickdraw_rectangle_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, roundrectangle, imagickdraw_roundrectangle_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, ellipse, imagickdraw_ellipse_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, skewx, imagickdraw_skewx_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, skewy, imagickdraw_skewy_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, translate, imagickdraw_translate_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, line, imagickdraw_line_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, arc, imagickdraw_arc_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, matte, imagickdraw_matte_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, polygon, imagickdraw_polygon_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, point, imagickdraw_point_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, gettextdecoration, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, gettextencoding, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setfontstretch, imagickdraw_setfontstretch_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setstrokeantialias, imagickdraw_setstrokeantialias_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, settextalignment, imagickdraw_setfontalignment_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, settextdecoration, imagickdraw_settextdecoration_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, settextundercolor, imagickdraw_settextundercolor_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setviewbox, imagickdraw_setviewbox_args, ZEND_ACC_PUBLIC)
  	PHP_ME(imagickdraw, clone, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, affine, imagickdraw_affine_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, bezier, imagickdraw_bezier_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, composite, imagickdraw_composite_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, color, imagickdraw_color_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, comment, imagickdraw_comment_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getclippath, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getcliprule, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getclipunits, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getfillcolor, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getfillopacity, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getfillrule, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getgravity, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getstrokeantialias, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getstrokecolor, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getstrokedasharray, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getstrokedashoffset, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getstrokelinecap, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getstrokelinejoin, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getstrokemiterlimit, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getstrokeopacity, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getstrokewidth, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, gettextalignment, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, gettextantialias, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, getvectorgraphics, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, gettextundercolor, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathclose, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathcurvetoabsolute, imagickdraw_pathcurvetoabsolute_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathcurvetorelative, imagickdraw_pathcurvetorelative_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathcurvetoquadraticbezierabsolute, imagickdraw_pathcurvetoquadraticbezierabsolute_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathcurvetoquadraticbezierrelative, imagickdraw_pathcurvetoquadraticbezierrelative_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathcurvetoquadraticbeziersmoothabsolute, imagickdraw_pathcurvetoquadraticbeziersmoothabsolute_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathcurvetoquadraticbeziersmoothrelative, imagickdraw_pathcurvetoquadraticbeziersmoothrelative_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathcurvetosmoothabsolute, imagickdraw_pathcurvetosmoothabsolute_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathcurvetosmoothrelative, imagickdraw_pathcurvetosmoothrelative_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathellipticarcabsolute, imagickdraw_pathellipticarcabsolute_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathellipticarcrelative, imagickdraw_pathellipticarcrelative_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathfinish, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathlinetoabsolute, imagickdraw_pathlinetoabsolute_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathlinetorelative, imagickdraw_pathlinetorelative_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathlinetohorizontalabsolute, imagickdraw_pathlinetohorizontalabsolute_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathlinetohorizontalrelative, imagickdraw_pathlinetohorizontalrelative_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathlinetoverticalabsolute, imagickdraw_pathlinetoverticalabsolute_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathlinetoverticalrelative, imagickdraw_pathlinetoverticalrelative_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathmovetoabsolute, imagickdraw_pathmovetoabsolute_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathmovetorelative, imagickdraw_pathmovetorelative_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pathstart, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, polyline, imagickdraw_polyline_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, popclippath, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, popdefs, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, poppattern, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pushclippath, imagickdraw_pushclippath_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pushdefs, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pushpattern, imagickdraw_pushpattern_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, render, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, resetvectorgraphics, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, rotate, imagickdraw_rotate_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, scale, imagickdraw_scale_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setclippath, imagickdraw_setclippath_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setcliprule, imagickdraw_setcliprule_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setclipunits, imagickdraw_setclipunits_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setfillopacity, imagickdraw_setfillopacity_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setfillpatternurl, imagickdraw_setfillpatternurl_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setfillrule, imagickdraw_setfillrule_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setgravity, imagickdraw_setgravity_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setstrokepatternurl, imagickdraw_setstrokepatternurl_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setstrokedashoffset, imagickdraw_setstrokedashoffset_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setstrokelinecap, imagickdraw_setstrokelinecap_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setstrokelinejoin, imagickdraw_setstrokelinejoin_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setstrokemiterlimit, imagickdraw_setstrokemiterlimit_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setstrokeopacity, imagickdraw_setstrokeopacity_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setvectorgraphics, imagickdraw_setvectorgraphics_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, popdrawingwand, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, pushdrawingwand, imagickdraw_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickdraw, setstrokedasharray, imagickdraw_setstrokedasharray_args, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixeliterator_zero_args, 0, 0, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixeliterator_setiteratorrow_args, 0, 0, 1)
		ZEND_ARG_INFO(0, row)
	ZEND_END_ARG_INFO()

static function_entry php_imagickpixeliterator_class_methods[] =
{
	PHP_ME(imagickpixeliterator, __construct, imagickpixeliterator_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixeliterator, newpixeliterator, imagickpixeliterator_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixeliterator, newpixelregioniterator, imagickpixeliterator_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixeliterator, getiteratorrow, imagickpixeliterator_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixeliterator, setiteratorrow, imagickpixeliterator_setiteratorrow_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixeliterator, setiteratorfirstrow, imagickpixeliterator_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixeliterator, setiteratorlastrow, imagickpixeliterator_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixeliterator, getpreviousiteratorrow, imagickpixeliterator_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixeliterator, getcurrentiteratorrow, imagickpixeliterator_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixeliterator, getnextiteratorrow, imagickpixeliterator_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixeliterator, resetiterator, imagickpixeliterator_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixeliterator, synciterator, imagickpixeliterator_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixeliterator, destroy, imagickpixeliterator_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixeliterator, clear, imagickpixeliterator_zero_args, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixel_zero_args, 0, 0, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixel_setcolor_args, 0, 0, 1)
		ZEND_ARG_INFO(0, color)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixel_setalpha_args, 0, 0, 1)
		ZEND_ARG_INFO(0, alpha)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixel_issimilar_args, 0, 0, 1)
		ZEND_ARG_OBJ_INFO(0, imagickpixel, imagickpixel, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixel_setblack_args, 0, 0, 1)
		ZEND_ARG_INFO(0, black)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixel_setblue_args, 0, 0, 1)
		ZEND_ARG_INFO(0, blue)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixel_setcyan_args, 0, 0, 1)
		ZEND_ARG_INFO(0, cyan)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixel_setgreen_args, 0, 0, 1)
		ZEND_ARG_INFO(0, green)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixel_setred_args, 0, 0, 1)
		ZEND_ARG_INFO(0, red)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixel_setyellow_args, 0, 0, 1)
		ZEND_ARG_INFO(0, yellow)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixel_setmagenta_args, 0, 0, 1)
		ZEND_ARG_INFO(0, magenta)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixel_setopacity_args, 0, 0, 1)
		ZEND_ARG_INFO(0, opacity)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixel_setfuzz_args, 0, 0, 1)
		ZEND_ARG_INFO(0, fuzz)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagickpixel_sethsl_args, 0, 0, 3)
		ZEND_ARG_INFO(0, hue)
		ZEND_ARG_INFO(0, saturation)
		ZEND_ARG_INFO(0, luminosity)
	ZEND_END_ARG_INFO()

static function_entry php_imagickpixel_class_methods[] =
{
	PHP_ME(imagickpixel, __construct, imagickpixel_zero_args, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(imagickpixel, setcolor, imagickpixel_setcolor_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, setalpha, imagickpixel_setalpha_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, getalpha, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, clear, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, destroy, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, issimilar, imagickpixel_issimilar_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, getblack, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, setblack, imagickpixel_setblack_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, getblue, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, setblue, imagickpixel_setblue_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, getcyan, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, setcyan, imagickpixel_setcyan_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, getgreen, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, setgreen, imagickpixel_setgreen_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, getred, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, setred, imagickpixel_setred_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, getyellow, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, setyellow, imagickpixel_setyellow_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, getmagenta, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, setmagenta, imagickpixel_setmagenta_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, getopacity, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, setopacity, imagickpixel_setopacity_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, getfuzz, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, setfuzz, imagickpixel_setfuzz_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, gethsl, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, sethsl, imagickpixel_sethsl_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, getcolorasstring, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, getcolorasnormalizedstring, imagickpixel_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagickpixel, getcolorcount, imagickpixel_zero_args, ZEND_ACC_PUBLIC)

	{ NULL, NULL, NULL }
};

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_zero_args, 0, 0, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_readimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, filename)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_readimageblob_args, 0, 0, 1)
		ZEND_ARG_INFO(0, imageContents)
		ZEND_ARG_INFO(0, fileName)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimageformat_args, 0, 0, 1)
		ZEND_ARG_INFO(0, imageFormat)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_scaleimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, width)
		ZEND_ARG_INFO(0, height)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_writeimage_args, 0, 0, 0)
		ZEND_ARG_INFO(0, fileName)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_writeimages_args, 0, 0, 2)
		ZEND_ARG_INFO(0, fileName)
		ZEND_ARG_INFO(0, adjoin)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_blurimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_blurimagechannel_args, 0, 0, 3)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_thumbnailimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, width)
		ZEND_ARG_INFO(0, height)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagefilename_args, 0, 0, 1)
		ZEND_ARG_INFO(0, fileName)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimageindex_args, 0, 0, 1)
		ZEND_ARG_INFO(0, index)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_commentimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, comment)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_cropimage_args, 0, 0, 4)
		ZEND_ARG_INFO(0, width)
		ZEND_ARG_INFO(0, height)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_labelimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, label)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_drawimage_args, 0, 0, 1)
		ZEND_ARG_OBJ_INFO(0, ImagickDraw, ImagickDraw, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagecompressionquality_args, 0, 0, 1)
		ZEND_ARG_INFO(0, quality)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_annotateimage_args, 0, 0, 5)
		ZEND_ARG_OBJ_INFO(0, ImagickDraw, ImagickDraw, 0)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
		ZEND_ARG_INFO(0, angle)
		ZEND_ARG_INFO(0, text)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_compositeimage_args, 0, 0, 4)
		ZEND_ARG_OBJ_INFO(0, Imagick, Imagick, 0)
		ZEND_ARG_INFO(0, COMPOSITE)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_compositeimagechannel_args, 0, 0, 5)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_OBJ_INFO(0, Imagick, Imagick, 0)
		ZEND_ARG_INFO(0, COMPOSITE)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()


static
	ZEND_BEGIN_ARG_INFO_EX(imagick_modulateimage_args, 0, 0, 3)
		ZEND_ARG_INFO(0, brightness)
		ZEND_ARG_INFO(0, saturation)
		ZEND_ARG_INFO(0, hue)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_montageimage_args, 0, 0, 5)
		ZEND_ARG_OBJ_INFO(0, ImagickDraw, ImagickDraw, 0)
		ZEND_ARG_INFO(0, tileGeometry)
		ZEND_ARG_INFO(0, thumbnailGeometry)
		ZEND_ARG_INFO(0, MONTAGEMODE)
		ZEND_ARG_INFO(0, frame)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_thresholdimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, threshold)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_thresholdimagechannel_args, 0, 0, 2)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, threshold)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_randomthresholdimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, low)
		ZEND_ARG_INFO(0, high)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_randomthresholdimagechannel_args, 0, 0, 3)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, low)
		ZEND_ARG_INFO(0, high)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_adaptivethresholdimage_args, 0, 0, 3)
		ZEND_ARG_INFO(0, width)
		ZEND_ARG_INFO(0, height)
		ZEND_ARG_INFO(0, offset)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_blackthresholdimage_args, 0, 0, 1)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_whitethresholdimage_args, 0, 0, 1)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_appendimages_args, 0, 0, 1)
		ZEND_ARG_INFO(0, stack)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_charcoalimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_normalizeimagechannel_args, 0, 0, 1)
		ZEND_ARG_INFO(0, CHANNEL)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_oilpaintimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, radius)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_posterizeimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, levels)
		ZEND_ARG_INFO(0, dither)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_radialblurimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, angle)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_radialblurimagechannel_args, 0, 0, 2)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, angle)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_raiseimage_args, 0, 0, 5)
		ZEND_ARG_INFO(0, width)
		ZEND_ARG_INFO(0, height)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
		ZEND_ARG_INFO(0, raise)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_resampleimage_args, 0, 0, 4)
		ZEND_ARG_INFO(0, xResolution)
		ZEND_ARG_INFO(0, yResolution)
		ZEND_ARG_INFO(0, FILTER)
		ZEND_ARG_INFO(0, blur)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_resizeimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_rollimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_rotateimage_args, 0, 0, 2)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
		ZEND_ARG_INFO(0, degrees)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_sampleimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, columns)
		ZEND_ARG_INFO(0, rows)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_solarizeimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, threshold)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_shadeimage_args, 0, 0, 3)
		ZEND_ARG_INFO(0, gray)
		ZEND_ARG_INFO(0, azimuth)
		ZEND_ARG_INFO(0, elevation)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_shadowimage_args, 0, 0, 4)
		ZEND_ARG_INFO(0, opacity)
		ZEND_ARG_INFO(0, sigma)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimageattribute_args, 0, 0, 2)
		ZEND_ARG_INFO(0, key)
		ZEND_ARG_INFO(0, value)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagebackgroundcolor_args, 0, 0, 1)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagecompose_args, 0, 0, 1)
		ZEND_ARG_INFO(0, COMPOSITE)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagecompression_args, 0, 0, 1)
		ZEND_ARG_INFO(0, COMPRESSION)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagedelay_args, 0, 0, 1)
		ZEND_ARG_INFO(0, delay)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagedepth_args, 0, 0, 1)
		ZEND_ARG_INFO(0, depth)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagegamma_args, 0, 0, 1)
		ZEND_ARG_INFO(0, gamma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimageiterations_args, 0, 0, 1)
		ZEND_ARG_INFO(0, iterations)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagematte_args, 0, 0, 1)
		ZEND_ARG_INFO(0, enable)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagemattecolor_args, 0, 0, 1)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagepage_args, 0, 0, 4)
		ZEND_ARG_INFO(0, width)
		ZEND_ARG_INFO(0, height)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimageresolution_args, 0, 0, 2)
		ZEND_ARG_INFO(0, xResolution)
		ZEND_ARG_INFO(0, yResolution)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagescene_args, 0, 0, 1)
		ZEND_ARG_INFO(0, scene)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagetickspersecond_args, 0, 0, 1)
		ZEND_ARG_INFO(0, ticksPerSecond)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagetype_args, 0, 0, 1)
		ZEND_ARG_INFO(0, IMGTYPE)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimageunits_args, 0, 0, 1)
		ZEND_ARG_INFO(0, RESOLUTION)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_sharpenimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_sharpenimagechannel_args, 0, 0, 3)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_shaveimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, columns)
		ZEND_ARG_INFO(0, rows)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_shearimage_args, 0, 0, 3)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
		ZEND_ARG_INFO(0, xShear)
		ZEND_ARG_INFO(0, yShear)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_sketchimage_args, 0, 0, 3)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
		ZEND_ARG_INFO(0, angle)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_spliceimage_args, 0, 0, 4)
		ZEND_ARG_INFO(0, width)
		ZEND_ARG_INFO(0, height)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_pingimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, fileName)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_pingimageblob_args, 0, 0, 1)
		ZEND_ARG_INFO(0, imageContents)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_readimagefile_args, 0, 0, 1)
		ZEND_ARG_INFO(0, fp)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_pingimagefile_args, 0, 0, 1)
		ZEND_ARG_INFO(0, fp)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_spreadimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, radius)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_swirlimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, degrees)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_trimimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, fuzz)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_getpixelregioniterator_args, 0, 0, 5)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
		ZEND_ARG_INFO(0, columns)
		ZEND_ARG_INFO(0, rows)
		ZEND_ARG_INFO(0, modify)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_waveimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, amplitude)
		ZEND_ARG_INFO(0, waveLenght)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_vignetteimage_args, 0, 0, 4)
		ZEND_ARG_INFO(0, blackPoint)
		ZEND_ARG_INFO(0, whitePoint)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_steganoimage_args, 0, 0, 2)
		ZEND_ARG_OBJ_INFO(0, Imagick, Imagick, 0)
		ZEND_ARG_INFO(0, offset)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX( imagick_addnoiseimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, NOISE)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX( imagick_addnoiseimagechannel_args, 0, 0, 2)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, NOISE)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX( imagick_adaptiveresizeimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, columns)
		ZEND_ARG_INFO(0, rows)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_adaptivesharpenimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_adaptivesharpenimagechannel_args, 0, 0, 3)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_motionblurimage_args, 0, 0, 3)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
		ZEND_ARG_INFO(0, angle)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_morphimages_args, 0, 0, 1)
		ZEND_ARG_INFO(0, frames)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_affinetransformimage_args, 0, 0, 1)
		ZEND_ARG_OBJ_INFO(0, ImagickDraw, ImagickDraw, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_borderimage_args, 0, 0, 3)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
		ZEND_ARG_INFO(0, width)
		ZEND_ARG_INFO(0, height)
	ZEND_END_ARG_INFO()


static
	ZEND_BEGIN_ARG_INFO_EX(imagick_chopimage_args, 0, 0, 4)
		ZEND_ARG_INFO(0, width)
		ZEND_ARG_INFO(0, height)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_clippathimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, pathname)
		ZEND_ARG_INFO(0, inside)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_colorfloodfillimage_args, 0, 0, 5)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
		ZEND_ARG_INFO(0, fuzz)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
		ZEND_ARG_INFO(0, y)
		ZEND_ARG_INFO(0, x)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_colorizeimage_args, 0, 0, 2)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_compareimagechannels_args, 0, 0, 3)
		ZEND_ARG_OBJ_INFO(0, Imagick, Imagick, 0)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_OBJ_INFO(0, Imagick, Imagick, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_compareimagelayers_args, 0, 0, 1)
		ZEND_ARG_INFO(0, LAYER)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_compareimages_args, 0, 0, 2)
		ZEND_ARG_OBJ_INFO(0, Imagick, Imagick, 0)
		ZEND_ARG_INFO(0, METRIC)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_contrastimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, sharpen)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_contraststretchimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, blackPoint)
		ZEND_ARG_INFO(0, whitePoint)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_contraststretchimagechannel_args, 0, 0, 3)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, blackPoint)
		ZEND_ARG_INFO(0, whitePoint)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_convolveimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, kernel)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_convolveimagechannel_args, 0, 0, 2)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, kernel)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_cyclecolormapimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, displace)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_edgeimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, radius)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_embossimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_evaluateimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, EVALUATE)
		ZEND_ARG_INFO(0, constant)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_evaluateimagechannel_args, 0, 0, 3)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, EVALUATE)
		ZEND_ARG_INFO(0, constant)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_adaptiveblurimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_adaptiveblurimagechannel_args, 0, 0, 3)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_frameimage_args, 0, 0, 5)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
		ZEND_ARG_INFO(0, width)
		ZEND_ARG_INFO(0, height)
		ZEND_ARG_INFO(0, innerBevel)
		ZEND_ARG_INFO(0, outerBevel)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_fximage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, expression)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_fximagechannel_args, 0, 0, 2)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, expression)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_gammaimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, gamma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_gammaimagechannel_args, 0, 0, 2)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, gamma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_gaussianblurimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_gaussianblurimagechannel_args, 0, 0, 3)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_getimagechanneldepth_args, 0, 0, 1)
		ZEND_ARG_INFO(0, CHANNEL)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_getimagechanneldistortion_args, 0, 0, 3)
		ZEND_ARG_OBJ_INFO(0, Imagick, Imagick, 0)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, METRIC)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_getimagechannelextrema_args, 0, 0, 1)
		ZEND_ARG_INFO(0, CHANNEL)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_getimagechannelmean_args, 0, 0, 1)
		ZEND_ARG_INFO(0, CHANNEL)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_getimagecolormapcolor_args, 0, 0, 1)
		ZEND_ARG_INFO(0, index)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_getimagedistortion_args, 0, 0, 2)
		ZEND_ARG_OBJ_INFO(0, Imagick, Imagick, 0)
		ZEND_ARG_INFO(0, METRIC)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_getimagepixelcolor_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_getimageprofile_args, 0, 0, 1)
		ZEND_ARG_INFO(0, name)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_getimagetotalinkdensity_args, 0, 0, 1)
		ZEND_ARG_INFO(0, radius)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_getimageregion_args, 0, 0, 4)
		ZEND_ARG_INFO(0, width)
		ZEND_ARG_INFO(0, height)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_levelimage_args, 0, 0, 3)
		ZEND_ARG_INFO(0, blackPoint)
		ZEND_ARG_INFO(0, gamma)
		ZEND_ARG_INFO(0, whitePoint)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_levelimagechannel_args, 0, 0, 4)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, blackPoint)
		ZEND_ARG_INFO(0, gamma)
		ZEND_ARG_INFO(0, whitePoint)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_linearstretchimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, blackPoint)
		ZEND_ARG_INFO(0, whitePoint)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_mapimage_args, 0, 0, 2)
		ZEND_ARG_OBJ_INFO(0, Imagick, Imagick, 0)
		ZEND_ARG_INFO(0, dither)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_mattefloodfillimage_args, 0, 0, 5)
		ZEND_ARG_INFO(0, alpha)
		ZEND_ARG_INFO(0, fuzz)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_medianfilterimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, radius)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_negateimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, gray)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_negateimagechannel_args, 0, 0, 2)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, gray)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_paintopaqueimage_args, 0, 0, 3)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
		ZEND_ARG_INFO(0, fuzz)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_paintopaqueimagechannel_args, 0, 0, 4)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
		ZEND_ARG_INFO(0, fuzz)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_painttransparentimage_args, 0, 0, 3)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
		ZEND_ARG_INFO(0, alpha)
		ZEND_ARG_INFO(0, fuzz)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_previewimages_args, 0, 0, 1)
		ZEND_ARG_INFO(0, PREVIEW)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_profileimage_args, 0, 0, 2)
		ZEND_ARG_INFO(0, name)
		ZEND_ARG_INFO(0, profile)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_quantizeimage_args, 0, 0, 5)
		ZEND_ARG_INFO(0, numColors)
		ZEND_ARG_INFO(0, COLORSPACE)
		ZEND_ARG_INFO(0, treeDepth)
		ZEND_ARG_INFO(0, dither)
		ZEND_ARG_INFO(0, measureError)
	ZEND_END_ARG_INFO()


static
	ZEND_BEGIN_ARG_INFO_EX(imagick_quantizeimages_args, 0, 0, 5)
		ZEND_ARG_INFO(0, numColors)
		ZEND_ARG_INFO(0, COLORSPACE)
		ZEND_ARG_INFO(0, treeDepth)
		ZEND_ARG_INFO(0, dither)
		ZEND_ARG_INFO(0, measureError)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_reducenoiseimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, radius)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_removeimageprofile_args, 0, 0, 1)
		ZEND_ARG_INFO(0, name)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_separateimagechannel_args, 0, 0, 1)
		ZEND_ARG_INFO(0, CHANNEL)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_sepiatoneimage_args, 0, 0, 1)
		ZEND_ARG_INFO(0, threshold)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagebias_args, 0, 0, 1)
		ZEND_ARG_INFO(0, bias)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimageblueprimary_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagebordercolor_args, 0, 0, 1)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagechanneldepth_args, 0, 0, 2)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, depth)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagecolormapcolor_args, 0, 0, 2)
		ZEND_ARG_INFO(0, index)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagecolorspace_args, 0, 0, 1)
		ZEND_ARG_INFO(0, COLORSPACE)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagedispose_args, 0, 0, 1)
		ZEND_ARG_INFO(0, DISPOSETYPE)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimageextent_args, 0, 0, 2)
		ZEND_ARG_INFO(0, columns)
		ZEND_ARG_INFO(0, rows)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagegreenprimary_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimageinterlacescheme_args, 0, 0, 1)
		ZEND_ARG_INFO(0, INTERLACE)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimageinterpolatemethod_args, 0, 0, 1)
		ZEND_ARG_INFO(0, INTERPOLATE)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimageprofile_args, 0, 0, 2)
		ZEND_ARG_INFO(0, name)
		ZEND_ARG_INFO(0, profile)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimageredprimary_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagerenderingintent_args, 0, 0, 1)
		ZEND_ARG_INFO(0, RENDERINGINTENT)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagevirtualpixelmethod_args, 0, 0, 1)
		ZEND_ARG_INFO(0, VIRTUALPIXELMETHOD)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setimagewhitepoint_args, 0, 0, 2)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_sigmoidalcontrastimage_args, 0, 0, 3)
		ZEND_ARG_INFO(0, sharpen)
		ZEND_ARG_INFO(0, alpha)
		ZEND_ARG_INFO(0, beta)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_sigmoidalcontrastimagechannel_args, 0, 0, 4)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, sharpen)
		ZEND_ARG_INFO(0, alpha)
		ZEND_ARG_INFO(0, beta)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_stereoimage_args, 0, 0, 1)
		ZEND_ARG_OBJ_INFO(0, Imagick, Imagick, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_textureimage_args, 0, 0, 1)
		ZEND_ARG_OBJ_INFO(0, Imagick, Imagick, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_tintimage_args, 0, 0, 2)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_unsharpmaskimage_args, 0, 0, 4)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
		ZEND_ARG_INFO(0, amount)
		ZEND_ARG_INFO(0, threshold)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_unsharpmaskimagechannel_args, 0, 0, 5)
		ZEND_ARG_INFO(0, CHANNEL)
		ZEND_ARG_INFO(0, radius)
		ZEND_ARG_INFO(0, sigma)
		ZEND_ARG_INFO(0, amount)
		ZEND_ARG_INFO(0, threshold)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_addimage_args, 0, 0, 4)
		ZEND_ARG_OBJ_INFO(0, Imagick, Imagick, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_newimage_args, 0, 0, 3)
		ZEND_ARG_INFO(0, columns)
		ZEND_ARG_INFO(0, rows)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_getoption_args, 0, 0, 1)
		ZEND_ARG_INFO(0, key)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_getresource_args, 0, 0, 1)
		ZEND_ARG_INFO(0, resourceType)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_getresourcelimit_args, 0, 0, 1)
		ZEND_ARG_INFO(0, resourceType)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setbackgroundcolor_args, 0, 0, 1)
		ZEND_ARG_OBJ_INFO(0, ImagickPixel, ImagickPixel, 0)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setcompression_args, 0, 0, 1)
		ZEND_ARG_INFO(0, compression)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setcompressionquality_args, 0, 0, 1)
		ZEND_ARG_INFO(0, compressionquality)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setfilename_args, 0, 0, 1)
		ZEND_ARG_INFO(0, filename)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setformat_args, 0, 0, 1)
		ZEND_ARG_INFO(0, format)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setinterlacescheme_args, 0, 0, 1)
		ZEND_ARG_INFO(0, INTERLACE)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setoption_args, 0, 0, 2)
		ZEND_ARG_INFO(0, key)
		ZEND_ARG_INFO(0, value)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setpage_args, 0, 0, 4)
		ZEND_ARG_INFO(0, width)
		ZEND_ARG_INFO(0, height)
		ZEND_ARG_INFO(0, x)
		ZEND_ARG_INFO(0, y)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setresourcelimit_args, 0, 0, 2)
		ZEND_ARG_INFO(0, RESOURCETYPE)
		ZEND_ARG_INFO(0, limit)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setresolution_args, 0, 0, 2)
		ZEND_ARG_INFO(0, xResolution)
		ZEND_ARG_INFO(0, yResolution)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setsamplingfactors_args, 0, 0, 1)
		ZEND_ARG_INFO(0, factors)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setsize_args, 0, 0, 2)
		ZEND_ARG_INFO(0, columns)
		ZEND_ARG_INFO(0, rows)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_setsizeoffset_args, 0, 0, 3)
		ZEND_ARG_INFO(0, columns)
		ZEND_ARG_INFO(0, rows)
		ZEND_ARG_INFO(0, offset)
	ZEND_END_ARG_INFO()

static
	ZEND_BEGIN_ARG_INFO_EX(imagick_settype_args, 0, 0, 1)
		ZEND_ARG_INFO(0, IMGTYPE)
	ZEND_END_ARG_INFO()




static function_entry php_imagick_class_methods[] =
{
	PHP_ME(imagick, __construct, imagick_zero_args, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(imagick, readimage, imagick_readimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, readimageblob, imagick_readimageblob_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimageformat, imagick_setimageformat_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, scaleimage, imagick_scaleimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, writeimage, imagick_writeimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, writeimages, imagick_writeimages_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, blurimage, imagick_blurimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, blurimagechannel, imagick_blurimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, thumbnailimage, imagick_thumbnailimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagefilename, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagefilename, imagick_setimagefilename_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimageformat, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, removeimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, destroy, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, clear, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, clone, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagesize, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimageblob, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setfirstiterator, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, resetiterator, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, previousimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, nextimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, haspreviousimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, hasnextimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimageindex, imagick_setimageindex_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimageindex, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, commentimage, imagick_commentimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, cropimage, imagick_cropimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, labelimage, imagick_labelimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagegeometry, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, drawimage, imagick_drawimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagecompressionquality, imagick_setimagecompressionquality_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, annotateimage, imagick_annotateimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, compositeimage, imagick_compositeimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, compositeimagechannel, imagick_compositeimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, modulateimage, imagick_modulateimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagecolors, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, montageimage, imagick_montageimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, identifyimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getpixeliterator, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, thresholdimage, imagick_thresholdimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, thresholdimagechannel, imagick_thresholdimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, randomthresholdimage, imagick_randomthresholdimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, randomthresholdimagechannel, imagick_randomthresholdimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, adaptivethresholdimage, imagick_adaptivethresholdimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, blackthresholdimage, imagick_blackthresholdimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, whitethresholdimage, imagick_whitethresholdimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, appendimages, imagick_appendimages_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, charcoalimage, imagick_charcoalimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, normalizeimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, normalizeimagechannel, imagick_normalizeimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, oilpaintimage, imagick_oilpaintimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, posterizeimage, imagick_posterizeimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, radialblurimage, imagick_radialblurimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, radialblurimagechannel, imagick_radialblurimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, raiseimage, imagick_raiseimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, resampleimage, imagick_resampleimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, resizeimage, imagick_resizeimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, rollimage, imagick_rollimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, rotateimage, imagick_rotateimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, sampleimage, imagick_sampleimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, solarizeimage, imagick_solarizeimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, shadeimage, imagick_shadeimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, shadowimage, imagick_shadowimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimageattribute, imagick_setimageattribute_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagebackgroundcolor, imagick_setimagebackgroundcolor_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagecompose, imagick_setimagecompose_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagecompression, imagick_setimagecompression_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagedelay, imagick_setimagedelay_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagedepth, imagick_setimagedepth_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagegamma, imagick_setimagegamma_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimageiterations, imagick_setimageiterations_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagematte, imagick_setimagematte_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagemattecolor, imagick_setimagemattecolor_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagepage, imagick_setimagepage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimageresolution, imagick_setimageresolution_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagescene, imagick_setimagescene_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagetickspersecond, imagick_setimagetickspersecond_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagetype, imagick_setimagetype_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimageunits, imagick_setimageunits_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, sharpenimage, imagick_sharpenimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, sharpenimagechannel, imagick_sharpenimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, shaveimage, imagick_shaveimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, shearimage, imagick_shearimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, sketchimage, imagick_sketchimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, spliceimage, imagick_spliceimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, pingimage, imagick_pingimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, pingimageblob, imagick_pingimageblob_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, readimagefile, imagick_readimagefile_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, pingimagefile, imagick_pingimagefile_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, spreadimage, imagick_spreadimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, swirlimage, imagick_swirlimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, stripimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, transposeimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, transverseimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, trimimage, imagick_trimimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, waveimage, imagick_waveimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, vignetteimage, imagick_vignetteimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, queryformats, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getpixelregioniterator, imagick_getpixelregioniterator_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, steganoimage, imagick_steganoimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, addnoiseimage, imagick_addnoiseimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, addnoiseimagechannel, imagick_addnoiseimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, adaptiveresizeimage, imagick_adaptiveresizeimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, adaptivesharpenimage, imagick_adaptivesharpenimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, adaptivesharpenimagechannel, imagick_adaptivesharpenimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, motionblurimage, imagick_motionblurimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, mosaicimages, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, morphimages, imagick_morphimages_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, minifyimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, affinetransformimage, imagick_affinetransformimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, averageimages, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, borderimage, imagick_borderimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, chopimage, imagick_chopimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, clipimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, clippathimage, imagick_clippathimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, coalesceimages, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, colorfloodfillimage, imagick_colorfloodfillimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, colorizeimage, imagick_colorizeimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, compareimagechannels, imagick_compareimagechannels_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, compareimagelayers, imagick_compareimagelayers_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, compareimages, imagick_compareimages_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, contrastimage, imagick_contrastimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, combineimages, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, contraststretchimage, imagick_contraststretchimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, contraststretchimagechannel, imagick_contraststretchimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, convolveimage, imagick_convolveimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, convolveimagechannel, imagick_convolveimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, cyclecolormapimage, imagick_cyclecolormapimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, deconstructimages, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, despeckleimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, edgeimage, imagick_edgeimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, embossimage, imagick_embossimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, enhanceimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, equalizeimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, evaluateimage, imagick_evaluateimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, evaluateimagechannel, imagick_evaluateimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, adaptiveblurimage, imagick_adaptiveblurimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, adaptiveblurimagechannel, imagick_adaptiveblurimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, flattenimages, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, flipimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, flopimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, frameimage, imagick_frameimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, fximage, imagick_fximage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, fximagechannel, imagick_fximagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, gammaimage, imagick_gammaimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, gammaimagechannel, imagick_gammaimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, gaussianblurimage, imagick_gaussianblurimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, gaussianblurimagechannel, imagick_gaussianblurimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagebackgroundcolor, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimageblueprimary, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagebordercolor, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagechanneldepth, imagick_getimagechanneldepth_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagechanneldistortion, imagick_getimagechanneldistortion_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagechannelextrema, imagick_getimagechannelextrema_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagechannelmean, imagick_getimagechannelmean_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagechannelstatistics, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagecolormapcolor, imagick_getimagecolormapcolor_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagecolorspace, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagecompose, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagedelay, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagedepth, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagedistortion, imagick_getimagedistortion_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimageextrema, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagedispose, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagegamma, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagegreenprimary, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimageheight, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagehistogram, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimageinterlacescheme, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimageinterpolatemethod, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimageiterations, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagematte, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagemattecolor, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagepage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagepixelcolor, imagick_getimagepixelcolor_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimageprofile, imagick_getimageprofile_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimageredprimary, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagerenderingintent, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimageresolution, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagescene, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagesignature, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagetickspersecond, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagetype, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimageunits, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagevirtualpixelmethod, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagewhitepoint, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagewidth, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getnumberimages, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimagetotalinkdensity, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimageregion, imagick_getimageregion_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, implodeimage, imagick_getimagetotalinkdensity_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, levelimage, imagick_levelimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, levelimagechannel, imagick_levelimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, linearstretchimage, imagick_linearstretchimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, magnifyimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, mapimage, imagick_mapimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, mattefloodfillimage, imagick_mattefloodfillimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, medianfilterimage, imagick_medianfilterimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, negateimage, imagick_negateimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, negateimagechannel, imagick_negateimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, paintopaqueimage, imagick_paintopaqueimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, paintopaqueimagechannel, imagick_paintopaqueimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, optimizeimagelayers, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, painttransparentimage, imagick_painttransparentimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, previewimages, imagick_previewimages_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, profileimage, imagick_profileimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, quantizeimage, imagick_quantizeimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, quantizeimages, imagick_quantizeimages_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, reducenoiseimage, imagick_reducenoiseimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, removeimageprofile, imagick_removeimageprofile_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, separateimagechannel, imagick_separateimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, sepiatoneimage, imagick_sepiatoneimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagebias, imagick_setimagebias_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimageblueprimary, imagick_setimageblueprimary_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagebordercolor, imagick_setimagebordercolor_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagechanneldepth, imagick_setimagechanneldepth_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagecolormapcolor, imagick_setimagecolormapcolor_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagecolorspace, imagick_setimagecolorspace_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagedispose, imagick_setimagedispose_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimageextent, imagick_setimageextent_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagegreenprimary, imagick_setimagegreenprimary_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimageinterlacescheme, imagick_setimageinterlacescheme_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimageinterpolatemethod, imagick_setimageinterpolatemethod_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimageprofile, imagick_setimageprofile_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimageredprimary, imagick_setimageredprimary_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagerenderingintent, imagick_setimagerenderingintent_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagevirtualpixelmethod, imagick_setimagevirtualpixelmethod_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setimagewhitepoint, imagick_setimagewhitepoint_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, sigmoidalcontrastimage, imagick_sigmoidalcontrastimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, sigmoidalcontrastimagechannel, imagick_sigmoidalcontrastimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, stereoimage, imagick_stereoimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, textureimage, imagick_textureimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, tintimage, imagick_tintimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, uniqueimagecolors, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, unsharpmaskimage, imagick_unsharpmaskimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, unsharpmaskimagechannel, imagick_unsharpmaskimagechannel_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, addimage, imagick_addimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, newimage, imagick_newimage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getcompression, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getcompressionquality, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getcopyright, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getfilename, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getformat, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, gethomeurl, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getinterlacescheme, imagick_zero_args, ZEND_ACC_PUBLIC)

	PHP_ME(imagick, getoption, imagick_getoption_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getpackagename, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getpage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getquantumdepth, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getquantumrange, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getreleasedate, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getresource, imagick_getresource_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getresourcelimit, imagick_getresourcelimit_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getsamplingfactors, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getsize, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getsizeoffset, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, getversion, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setbackgroundcolor, imagick_setbackgroundcolor_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setcompression, imagick_setcompression_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setcompressionquality, imagick_setcompressionquality_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setfilename, imagick_setfilename_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setformat, imagick_setformat_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setinterlacescheme, imagick_setinterlacescheme_args, ZEND_ACC_PUBLIC)

	PHP_ME(imagick, setoption, imagick_setoption_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setpage, imagick_setpage_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setresourcelimit, imagick_setresourcelimit_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setresolution, imagick_setresolution_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setsamplingfactors, imagick_setsamplingfactors_args, ZEND_ACC_PUBLIC)


	PHP_ME(imagick, setsize, imagick_setsize_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, setsizeoffset, imagick_setsizeoffset_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, settype, imagick_settype_args, ZEND_ACC_PUBLIC)




	PHP_MALIAS(imagick, key, getimageindex, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_MALIAS(imagick, next, nextimage, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_MALIAS(imagick, rewind, setfirstiterator, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, valid, imagick_zero_args, ZEND_ACC_PUBLIC)
	PHP_ME(imagick, current, imagick_zero_args, ZEND_ACC_PUBLIC)

	{ NULL, NULL, NULL }
};

long getImageCount( MagickWand *magick_wand TSRMLS_DC)
{
	long images;

	if( !IsMagickWand( magick_wand ) || magick_wand == (MagickWand *)NULL )
	{
		return 0;
	}

	images = MagickGetNumberImages( magick_wand );
	return images;
}


void throwExceptionWithMessage( int type, char *description, long code TSRMLS_DC )
{
	switch( type )
	{
		case 1:
			zend_throw_exception( php_imagick_exception_class_entry, description, (long)code TSRMLS_CC);
			break;

		case 2:
			zend_throw_exception( php_imagickdraw_exception_class_entry, description, (long)code TSRMLS_CC);
			break;

		case 3:
			zend_throw_exception( php_imagickpixeliterator_sc_entry, description, (long)code TSRMLS_CC);
			break;

		case 4:
			zend_throw_exception( php_imagickpixel_exception_class_entry, description, (long)code TSRMLS_CC);
			break;
	}
	description = (char *)NULL;
}


void throwImagickException( MagickWand *magick_wand, long code TSRMLS_DC)
{
	ExceptionType severity;
	char *description;
	int free = 1;
	description = MagickGetException( magick_wand, &severity );

	if ( strlen( description ) == 0 )
	{
		description = "Undefined exception";
		free = 0;
	}

	zend_throw_exception( php_imagick_exception_class_entry, description, (long)code TSRMLS_CC);

	if( free == 1 )
	{
		IMAGICK_FREE_MEMORY( char *, description );
		MagickClearException( magick_wand );
	}
	description = (char *)NULL;
}


void throwImagickDrawException( DrawingWand *drawing_wand, long code TSRMLS_DC)
{
	ExceptionType severity;
	char *description;
	int free = 1;
	description = DrawGetException( drawing_wand, &severity );

	if ( strlen( description ) == 0 )
	{
		description = "Undefined exception";
		free = 0;
	}

	zend_throw_exception( php_imagickdraw_exception_class_entry, description, (long)code TSRMLS_CC);

	if( free == 1 )
	{
		IMAGICK_FREE_MEMORY( char *, description );
		DrawClearException( drawing_wand );
	}
	description = (char *)NULL;
}


void throwImagickPixelIteratorException( PixelIterator *pixel_iterator, long code TSRMLS_DC)
{
	ExceptionType severity;
	char *description;
	int free = 1;
	description = PixelGetIteratorException( pixel_iterator, &severity );

	if ( strlen( description ) == 0 )
	{
		description = "Undefined exception";
		free = 0;
	}

	zend_throw_exception( php_imagickpixeliterator_sc_entry, description, (long)code TSRMLS_CC);

	if( free == 1 )
	{
		IMAGICK_FREE_MEMORY( char *, description );
		PixelClearIteratorException( pixel_iterator );
	}
	description = (char *)NULL;
}


void throwImagickPixelException( PixelWand *pixel_wand, long code TSRMLS_DC)
{
	ExceptionType severity;
	char *description;
	int free = 1;
	description = PixelGetException( pixel_wand, &severity );

	if ( strlen( description ) == 0 )
	{
		description = "Undefined exception";
		free = 0;
	}

	zend_throw_exception( php_imagickpixel_exception_class_entry, description, (long)code TSRMLS_CC);

	if( free == 1 )
	{
		IMAGICK_FREE_MEMORY( char *, description );
		PixelClearException( pixel_wand );
	}
	description = (char *)NULL;
}


char *getHashValue( HashTable *hashTable TSRMLS_DC)
{
	char *tmpVal;
	zval **ppzval, tmpcopy;

	if ( zend_hash_get_current_data( hashTable, (void**)&ppzval ) == FAILURE )
	{
		tmpVal = "failure";
	}
	else
	{
		tmpcopy = **ppzval;
		zval_copy_ctor(&tmpcopy);

		INIT_PZVAL(&tmpcopy);
		convert_to_string(&tmpcopy);

		tmpVal = php_trim( Z_STRVAL( tmpcopy ), Z_STRLEN( tmpcopy ), (char *)NULL, 0, NULL, 3 TSRMLS_CC);

		zval_dtor(&tmpcopy);
		zend_hash_move_forward( hashTable );
	}
	return tmpVal;
}


int count_occurences_of( char needle, char *hayStack TSRMLS_DC )
{
	int occurances = 0;

	if (hayStack == (char *)NULL)
	{
		return 0;
	}

	while (*hayStack != '\0')
	{
		if (*(hayStack++) == needle)
		{
			occurances++;
		}
	}
	return occurances;
}

void add_assoc_string_helper( zval *retvalue, char *name, char *key, char *hashValue TSRMLS_DC )
{
	char *pch;
	char *trimmed;
	zval *array;
	int width, height;
	double x, y;

	if ( strncmp( hashValue, name, strlen( name ) ) == 0 )
	{
		if ( strcmp( "geometry" , key ) == 0 )
		{
			MAKE_STD_ZVAL(array);
			array_init(array);
			sscanf( hashValue, "%*s %d%*c%d", &width, &height );
			add_assoc_long( array, "width", width );
			add_assoc_long( array, "height", height );
			add_assoc_zval( retvalue, key, array );
		}

		else if( strcmp( "resolution" , key ) == 0 )
		{
			MAKE_STD_ZVAL(array);
			array_init(array);

			sscanf( hashValue, "%*s %lf%*c%lf", &x, &y );
			add_assoc_double( array, "x", x );
			add_assoc_double( array, "y", y );
			add_assoc_zval( retvalue, key, array );
		}
		else
		{
			pch = strchr( hashValue, ':' );
			pch = strchr( pch + 1, ' ' );
			trimmed = php_trim( pch, strlen( pch ), (char *)NULL, 0, NULL, 3 TSRMLS_CC);
			add_assoc_string( retvalue, key, trimmed, 1 );
			efree( trimmed );
		}
	}
}

double *getDoublesFromZval( zval *zArray, long *numElements TSRMLS_DC )
{
	zval **ppzval;
	HashTable *ht;
	double *dArray;
	long elements, i;

	*numElements = 0;
	elements = zend_hash_num_elements( Z_ARRVAL_P( zArray ) );

	if ( elements == 0 )
	{
		dArray = (double *)NULL;
		return dArray;
	}

	dArray = emalloc( sizeof( double ) * elements );
	ht = Z_ARRVAL_P( zArray );

	zend_hash_internal_pointer_reset( ht );

	for ( i = 0 ; i < elements ; i++ )
	{
		if ( zend_hash_get_current_data( ht, (void**)&ppzval ) == FAILURE )
		{
			efree( ht );
			dArray = (double *)NULL;
			return dArray;
		}

		if( Z_TYPE_PP( ppzval ) == IS_LONG )
		{
			dArray[i] = (double)Z_LVAL_PP( ppzval );
		}

		else if ( Z_TYPE_PP( ppzval ) == IS_DOUBLE )
		{
			dArray[i] = Z_DVAL_PP( ppzval );
		}

		else
		{
			efree( ht );
			dArray = (double *)NULL;
			return dArray;
		}
		zend_hash_move_forward( ht );
	}
	*numElements = elements;
	return dArray;
}

/* {{{ proto Imagick::__construct()
   The Imagick constructor
*/
PHP_METHOD(imagick, __construct)
{
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::queryFormats( [string pattern] )
   Returns formats supported by ImageMagick
*/
PHP_METHOD(imagick, queryformats)
{
	char **supportedFormats;
	unsigned long numFormats = 0;
	char *pattern = "*";
	int i, patternLen = 1;

	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "|s", &pattern, &patternLen ) == FAILURE )
	{
		return;
	}

	supportedFormats = (char **) MagickQueryFormats( pattern, &numFormats );
	array_init( return_value );

	for( i = 0 ; i < numFormats ; i++ )
	{
		add_next_index_string( return_value, supportedFormats[i], 1 );
		IMAGICK_FREE_MEMORY( char *, supportedFormats[i] );
	}
	IMAGICK_FREE_MEMORY( char **, supportedFormats );
	return;
}
/* }}} */

/* {{{ proto Imagick::valid()
    Checks if the current item is valid
*/
PHP_METHOD(imagick, valid)
{
	php_imagick_object *intern;
	zval *object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	if( intern->next_out_of_bound > 0 )
	{
		RETURN_FALSE;
	}

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::current()
    Returns information of current image
*/
PHP_METHOD(imagick, current)
{
	zval *object;
	php_imagick_object *intern;

	long size, width, height;
	char *format;
	char *name;
	object = getThis();

	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	size = MagickGetImageSize( intern->magick_wand );
	width = MagickGetImageWidth( intern->magick_wand );
	height = MagickGetImageHeight( intern->magick_wand );
	format = MagickGetImageFormat( intern->magick_wand );
	name = MagickGetImageFilename( intern->magick_wand );

	array_init( return_value );
	add_assoc_string( return_value, "name", name, 1 );
	add_assoc_long( return_value, "size", size );
	add_assoc_string( return_value, "format", (char *)format, 1 );
	add_assoc_long( return_value, "width", width );
	add_assoc_long( return_value, "height", height );

	IMAGICK_FREE_MEMORY( char *, name );
	IMAGICK_FREE_MEMORY( char *, format );

	return;
}
/* }}} */

/* {{{ proto Imagick::readImage( string filename )
    Reads image from filename
*/
PHP_METHOD(imagick, readimage)
{
	char *fileName;
	int fileNameLen;
	zval *object;
	MagickBooleanType status;
	php_imagick_object *intern;
	char *absolute;


	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &fileName, &fileNameLen ) == FAILURE )
	{
		return;
	}

	/* Fix because magickwand doesnt want to take relative paths */
	absolute = expand_filepath( fileName, NULL TSRMLS_CC);

	IMAGICK_SAFE_MODE_CHECK( "Safe mode restricts user to read image: %s", absolute );

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	status = MagickReadImage( intern->magick_wand, absolute );
	efree( absolute );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::pingImage( string filename )
    This method can be used to query image width, height, size, and format without reading the whole image in to memory.
*/
PHP_METHOD(imagick, pingimage)
{
	char *fileName;
	int fileNameLen;
	zval *object;
	MagickBooleanType status;
	php_imagick_object *intern;
	char *absolute;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &fileName, &fileNameLen ) == FAILURE )
	{
		return;
	}

	/* Fix because magickwand doesnt want to take relative paths */
	absolute = expand_filepath( fileName, NULL TSRMLS_CC);

	IMAGICK_SAFE_MODE_CHECK( "Safe mode restricts user to read image: %s", absolute );

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	status = MagickPingImage( intern->magick_wand, absolute );
	efree( absolute );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::readImage( resource filehandle )
    Reads image from open filehandle
*/
PHP_METHOD(imagick, readimagefile)
{
	FILE *fp;
	char *fileName;
	int fileNameLen;
	zval *object;
	MagickBooleanType status;
	php_imagick_object *intern;
	zval *zstream;
	php_stream *stream;

	fileName = (char *)NULL;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|s", &zstream, &fileName, &fileNameLen) == FAILURE)
	{
		return;
	}

	php_stream_from_zval( stream, &zstream );

	if( php_stream_can_cast( stream, PHP_STREAM_AS_STDIO ) == FAILURE )
	{
		RETURN_FALSE;
	}

	if (php_stream_cast(stream, PHP_STREAM_AS_STDIO, (void*)&fp, REPORT_ERRORS) == FAILURE)
	{
		RETURN_FALSE;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	status = MagickReadImageFile( intern->magick_wand, fp );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	MagickSetImageFilename( intern->magick_wand, fileName );

	RETURN_TRUE
}
/* }}} */

/* {{{ proto Imagick::pingImageFile( resource filehandle )
    This method can be used to query image width, height, size, and format without reading the whole image to memory.
*/
PHP_METHOD(imagick, pingimagefile)
{
	FILE *fp;
	char *fileName;
	int fileNameLen;
	zval *object;
	MagickBooleanType status;
	php_imagick_object *intern;
	zval *zstream;
	php_stream *stream;

	fileName = (char *)NULL;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|s", &zstream, &fileName, &fileNameLen) == FAILURE)
	{
		return;
	}

	php_stream_from_zval( stream, &zstream );

	if( php_stream_can_cast( stream, PHP_STREAM_AS_STDIO ) == FAILURE )
	{
		RETURN_FALSE;
	}

	if (php_stream_cast(stream, PHP_STREAM_AS_STDIO, (void*)&fp, REPORT_ERRORS) == FAILURE)
	{
		RETURN_FALSE;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	status = MagickPingImageFile( intern->magick_wand, fp );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	MagickSetImageFilename( intern->magick_wand, fileName );

	RETURN_TRUE
}
/* }}} */

/* {{{ proto Imagick::readBlob( string image )
    Reads image from a binary string
*/
PHP_METHOD(imagick, readimageblob)
{
	char *imageString;
	char *fileName;
	long fileNameLen;
	size_t imageStringLen;
	zval *object;
	MagickBooleanType status;
	php_imagick_object *intern;

	fileName = (char *)NULL;

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &imageString, &imageStringLen, &fileName, &fileNameLen ) == FAILURE )
	{
		return;
	}

	if ( strlen( imageString ) == 0 )
	{
		throwExceptionWithMessage( 1, "Zero size image string passed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	status = MagickReadImageBlob( intern->magick_wand, imageString, imageStringLen );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	/* Even if filename is null we need to give a name here. Otherwise segfaults will happen */
	MagickSetImageFilename( intern->magick_wand, fileName );
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::pingImageBlob( string image )
	This method can be used to query image width, height, size, and format without reading the whole image to memory.
*/
PHP_METHOD(imagick, pingimageblob)
{
	char *imageString;
	size_t imageStringLen;
	zval *object;
	MagickBooleanType status;
	php_imagick_object *intern;

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &imageString, &imageStringLen ) == FAILURE )
	{
		return;
	}

	if ( strlen( imageString ) == 0 )
	{
		throwExceptionWithMessage( 1, "Zero size image string passed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	status = MagickPingImageBlob( intern->magick_wand, imageString, imageStringLen );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::blurImage( float radius, float sigma )
	Adds blur filter to image
*/
PHP_METHOD(imagick, blurimage)
{
	double radius, sigma;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &radius, &sigma ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickBlurImage( intern->magick_wand, radius, sigma );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::blurImageChannel( CHANNELTYPE, float radius, float sigma )
	Adds blur filter to image channel.
*/
PHP_METHOD(imagick, blurimagechannel)
{
	double radius, sigma;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	long channel;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ldd", &channel, &radius, &sigma ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickBlurImageChannel( intern->magick_wand, channel, radius, sigma );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::adaptiveBlurImage( float radius, float sigma )
	Adds adaptive blur filter to image.
*/
PHP_METHOD(imagick, adaptiveblurimage)
{

	double radius, sigma;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &radius, &sigma ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickAdaptiveBlurImage( intern->magick_wand, radius, sigma );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::adaptiveBlurImageChannel( CHANNELTYPE, float radius, float sigma )
	Adds adaptive blur filter to image channel.
*/
PHP_METHOD(imagick, adaptiveblurimagechannel)
{
	double radius, sigma;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	long channel;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ldd", &channel, &radius, &sigma ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickAdaptiveBlurImageChannel( intern->magick_wand, channel, radius, sigma );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;

}
/* }}} */

/* {{{ proto Imagick::waveImage( float amplitude, float length )
	Adds wave filter to the image.
*/
PHP_METHOD(imagick, waveimage)
{
	double amplitude, waveLength;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &amplitude, &waveLength ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickWaveImage( intern->magick_wand, amplitude, waveLength );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::vignetteImage( float blackPoint, float whitePoint, int x, int y )
	Adds vignette filter to the image
*/
PHP_METHOD(imagick, vignetteimage)
{
	double blackPoint, whitePoint;
	long x, y;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ddll", &blackPoint, &whitePoint, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickVignetteImage( intern->magick_wand, blackPoint, whitePoint, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::clear()
	Clears all resources associated to Imagick object
*/
PHP_METHOD(imagick, clear)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	status = IsMagickWand( intern->magick_wand );

	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}

	ClearMagickWand( intern->magick_wand );
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::destroy()
	Destroys the Imagick object
*/
PHP_METHOD(imagick, destroy)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	status = IsMagickWand( intern->magick_wand );

	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}

	intern->magick_wand = (MagickWand *) DestroyMagickWand( intern->magick_wand );
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::scaleImage( int cols, int rows )
	Scales the size of an image to the given dimensions. Passing zero as either of
	the arguments will preserve dimension while scaling.
*/
PHP_METHOD(imagick, scaleimage)
{
	long x, y;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	unsigned long imageY, imageX;
	double tmp;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ll", &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	if ( ( x == 0 ) && ( y == 0 ) )
	{
		throwExceptionWithMessage( 1, "Can't scale image to zero size.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	if ( ( x == 0 ) || ( y == 0 ) )
	{
		imageX = MagickGetImageWidth( intern->magick_wand );
		imageY = MagickGetImageHeight( intern->magick_wand );

		if( x == 0 )
		{
			tmp = (double)imageX / (double)imageY;
			x = tmp * (double)y;
		}

		if ( y == 0 )
		{
			tmp = (double)imageY / (double)imageX;
			y = tmp * (double)x;
		}
	}

	status = MagickScaleImage( intern->magick_wand, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::cropImage( int width, height, int x, int y )
	Extracts a region of the image.
*/
PHP_METHOD(imagick, cropimage)
{

	long width, height, x, y;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "llll", &width, &height, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickCropImage( intern->magick_wand, width, height, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::spreadImage( float radius )
	Special effects method that randomly displaces each pixel in a block defined by the radius parameter.
*/
PHP_METHOD(imagick, spreadimage)
{

	double radius;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &radius ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSpreadImage( intern->magick_wand, radius );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::swirlImage( float degrees )
	Swirls the pixels about the center of the image, where degrees indicates the sweep of the arc through which each pixel is moved. You get a more dramatic effect as the degrees move from 1 to 360.
*/
PHP_METHOD(imagick, swirlimage)
{
	double degrees;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &degrees ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSwirlImage( intern->magick_wand, degrees );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::stripImage()
	Strips an image of all profiles and comments.
*/
PHP_METHOD(imagick, stripimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickStripImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::stripImage()
	Strips an image of all profiles and comments.
*/
PHP_METHOD(imagick, transposeimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickTransposeImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::transverseImage()
	Creates a horizontal mirror image by reflecting the pixels around the central y-axis while rotating them 270-degrees.
*/
PHP_METHOD(imagick, transverseimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickTransverseImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::trimImage(double fuzz)
	Remove edges that are the background color from the image.
*/
PHP_METHOD(imagick, trimimage)
{
	double fuzz;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &fuzz ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickTrimImage( intern->magick_wand, fuzz );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::chopImage( int width, int height, int x, int y)
	Removes a region of an image and collapses the image to occupy the removed portion
*/
PHP_METHOD(imagick, chopimage)
{
	php_imagick_object *intern;
	zval *object;
	long width, height, x, y;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "llll", &width, &height, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickChopImage( intern->magick_wand, width, height, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;

}
/* }}} */

/* {{{ proto Imagick::clipImage()
	Clips along the first path from the 8BIM profile, if present.
*/
PHP_METHOD(imagick, clipimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );
	status = MagickClipImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::clipPathImage( string pathname, bool inside)
	Clips along the named paths from the 8BIM profile, if present. Later operations take effect inside the path.  Id may be a number if preceded with #, to work on a numbered path, e.g., "#1" to use the first path.
*/
PHP_METHOD(imagick, clippathimage)
{
	php_imagick_object *intern;
	zval *object;
	char *clipPath;
	int clipPathLen;
	zend_bool inside;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "sb", &clipPath, &clipPathLen, &inside ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickClipPathImage( intern->magick_wand, clipPath, inside );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;

}
/* }}} */

/* {{{ proto Imagick::coalesceImages()
	Composites a set of images while respecting any page offsets and disposal methods.  GIF, MIFF, and MNG animation sequences typically start with an image background and each subsequent image varies in size and offset.  returns a new sequence where each image in the sequence is the same size as the first and composited with the next image in the sequence.
*/
PHP_METHOD(imagick, coalesceimages)
{
	MagickWand *tmpWand;
	zval *object;
	php_imagick_object *intern, *intern_second;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickCoalesceImages( intern->magick_wand );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Coalesce image failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_second = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_second->magick_wand = tmpWand;
	return;

}
/* }}} */

/* {{{ proto Imagick::colorFloodfillImage( ImagickPixel fill, double fuzz, ImagickPixel bordercolor, int x, int y)
	Changes the color value of any pixel that matches target and is an immediate neighbor.
*/
PHP_METHOD(imagick, colorfloodfillimage)
{
	php_imagick_object *intern;
	php_imagickpixel_object *intern_fill, *intern_border;
	zval *object, *fillObj, *borderObj;
	long x, y;
	double fuzz;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 5 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "OdOll", &fillObj, php_imagickpixel_sc_entry, &fuzz,
																   &borderObj, php_imagickpixel_sc_entry, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_fill = (php_imagickpixel_object *)zend_object_store_get_object(fillObj TSRMLS_CC);
	intern_border = (php_imagickpixel_object *)zend_object_store_get_object(borderObj TSRMLS_CC);

	status = MagickColorFloodfillImage( intern->magick_wand, intern_fill->pixel_wand, fuzz, intern_border->pixel_wand, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::combineImages()
	Combines one or more images into a single image. The grayscale value of the pixels of each image in the sequence is assigned in order to the specified channels of the combined image. The typical ordering would be image 1 => Red, 2 => Green, 3 => Blue, etc.
*/
PHP_METHOD(imagick, combineimages)
{
	MagickWand *tmpWand;
	zval *object;
	php_imagick_object *intern, *intern_return;
	long channelType;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &channelType ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickCombineImages( intern->magick_wand, channelType );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Combine images failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	return;
}
/* }}} */

/* {{{ proto Imagick::getImage()
	Returns a new Imagick object with the current image sequence.
*/
PHP_METHOD(imagick, getimage)
{
	MagickWand *tmpWand;
	zval *object;
	php_imagick_object *intern, *intern_return;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickGetImage( intern->magick_wand );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Get image failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	return;
}
/* }}} */

/* {{{ proto Imagick::addImage( Imagick source )
	Adds new image to Imagick object from the current position of the source object.
*/
PHP_METHOD(imagick, addimage)
{
	php_imagick_object *intern, *intern_add;
	zval *object, *addObj;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "O", &addObj, php_imagick_sc_entry ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	intern_add = (php_imagick_object *)zend_object_store_get_object(addObj TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern_add->magick_wand, 1, 1 );
	status = MagickAddImage( intern->magick_wand, intern_add->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::newImage( int cols, int rows, ImagickPixel background )
	Creates a new image and associates ImagickPixel value as background color
*/
PHP_METHOD(imagick, newimage)
{
	php_imagick_object *intern;
	php_imagickpixel_object *internbg;
	zval *object, *bgObj;
	MagickBooleanType status;
	long columns, rows;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "llO", &columns, &rows, &bgObj, php_imagickpixel_sc_entry ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	internbg = (php_imagickpixel_object *)zend_object_store_get_object(bgObj TSRMLS_CC);
	status = MagickNewImage( intern->magick_wand, columns, rows, internbg->pixel_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::getImageTotalInkDensity()
	Gets the image total ink density.
*/
PHP_METHOD(imagick, getimagetotalinkdensity)
{
	zval *object;
	php_imagick_object *intern;
	double inkDensity;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	inkDensity = MagickGetImageTotalInkDensity( intern->magick_wand );
	RETVAL_DOUBLE( inkDensity );
}
/* }}} */

/* {{{ proto Imagick::implodeImage( float radius )
	Creates a new image that is a copy of an existing one with the image pixels "implode" by the specified percentage. It allocates the memory necessary for the new Image structure and returns a pointer to the new image.
*/
PHP_METHOD(imagick, implodeimage)
{
	php_imagick_object *intern;
	zval *object;
	double radius;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &radius ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickImplodeImage( intern->magick_wand, radius );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::levelImage( float blackPoint, float gamma, float whitePoint )
	Adjusts the levels of an image by scaling the colors falling between specified white and black points to the full available quantum range. The parameters provided represent the black, mid, and white points. The black point specifies the darkest color in the image. Colors darker than the black point are set to zero. Mid point specifies a gamma correction to apply to the image.  White point specifies the lightest color in the image. Colors brighter than the white point are set to the maximum quantum value.
*/
PHP_METHOD(imagick, levelimage)
{
	php_imagick_object *intern;
	zval *object;
	double blackPoint, gamma, whitePoint;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ddd", &blackPoint, &gamma, &whitePoint ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickLevelImage( intern->magick_wand, blackPoint, gamma, whitePoint );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::levelImageChannel( CHANNELTYPE channel, float blackPoint, float gamma, float whitePoint )
	Adjusts the levels of an image by scaling the colors falling between specified white and black points to the full available quantum range. The parameters provided represent the black, mid, and white points. The black point specifies the darkest color in the image. Colors darker than the black point are set to zero. Mid point specifies a gamma correction to apply to the image.  White point specifies the lightest color in the image. Colors brighter than the white point are set to the maximum quantum value.
*/
PHP_METHOD(imagick, levelimagechannel)
{
	php_imagick_object *intern;
	zval *object;
	long channel;
	double blackPoint, gamma, whitePoint;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "lddd", &channel, &blackPoint, &gamma, &whitePoint ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickLevelImageChannel( intern->magick_wand, channel, blackPoint, gamma, whitePoint );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::linearStretchImage( float blackPoint, float whitePoint)
	Stretches with saturation the image intensity.
*/
PHP_METHOD(imagick, linearstretchimage)
{
	php_imagick_object *intern;
	zval *object;
	double blackPoint, whitePoint;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &blackPoint, &whitePoint ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickLinearStretchImage( intern->magick_wand, blackPoint, whitePoint );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::magnifyImage()
	Is a convenience method that scales an image proportionally to twice its original size.
*/
PHP_METHOD(imagick, magnifyimage)
{
	zval *object;
	php_imagick_object *intern;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickMagnifyImage( intern->magick_wand );
		/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::cycleColormapImage(Imagick map, bool dither)
	Replaces the colors of an image with the closest color from a reference image.
*/
PHP_METHOD(imagick, mapimage)
{
	php_imagick_object *intern, *intern_map;
	zval *object, *mapObj;
	zend_bool dither;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "Ob", &mapObj, php_imagick_sc_entry, &dither ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_map = (php_imagick_object *)zend_object_store_get_object(mapObj TSRMLS_CC);
	status = MagickMapImage( intern->magick_wand, intern_map->magick_wand, dither );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::matteFloodfillImage( float alpha,float fuzz,ImagickPixel bordercolor, int x, int y)
	Changes the transparency value of any pixel that matches target and is an immediate neighbor.  If the method FillToBorderMethod is specified, the transparency value is changed for any neighbor pixel that does not match the bordercolor member of image.
*/
PHP_METHOD(imagick, mattefloodfillimage)
{
	php_imagick_object *intern;
	php_imagickpixel_object *intern_border;
	zval *object, *borderObj;
	long x, y;
	double alpha, fuzz;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 5 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ddOll", &alpha, &fuzz, &borderObj, php_imagickpixel_sc_entry, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_border = (php_imagickpixel_object *)zend_object_store_get_object(borderObj TSRMLS_CC);
	status = MagickMatteFloodfillImage( intern->magick_wand, alpha, fuzz, intern_border->pixel_wand, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::medianFilterImage(float radius)
	Applies a digital filter that improves the quality of a noisy image.  Each pixel is replaced by the median in a set of neighboring pixels as defined by radius.
*/
PHP_METHOD(imagick, medianfilterimage)
{
	php_imagick_object *intern;
	zval *object;
	double radius;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &radius ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickMedianFilterImage( intern->magick_wand, radius );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::negateImage( bool gray )
	Negates the colors in the reference image.  The Grayscale option means that only grayscale values within the image are negated.
*/
PHP_METHOD(imagick, negateimage)
{
	php_imagick_object *intern;
	zval *object;
	zend_bool gray;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "b", &gray ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickNegateImage( intern->magick_wand, gray );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::negateImageChannel( CHANNELTYPE channel, bool gray )
	Negates the colors in the reference image.  The Grayscale option means that only grayscale values within the image are negated.
*/
PHP_METHOD(imagick, negateimagechannel)
{
	php_imagick_object *intern;
	zval *object;
	long channel;
	zend_bool gray;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "lb", &channel, &gray ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickNegateImageChannel( intern->magick_wand, channel, gray );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::paintOpaqueImage( ImagickPixel target, ImagickPixel fill, float fuzz)
	Changes any pixel that matches color with the color defined by fill.
*/
PHP_METHOD(imagick, paintopaqueimage)
{
	php_imagick_object *intern;
	php_imagickpixel_object *intern_fill, *intern_target;
	zval *object, *targetObj, *fillObj;
	double fuzz;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "OOd", &targetObj, php_imagickpixel_sc_entry, &fillObj, php_imagickpixel_sc_entry, &fuzz ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_target = (php_imagickpixel_object *)zend_object_store_get_object(targetObj TSRMLS_CC);
	intern_fill = (php_imagickpixel_object *)zend_object_store_get_object(fillObj TSRMLS_CC);

	status = MagickPaintOpaqueImage( intern->magick_wand, intern_target->pixel_wand, intern_fill->pixel_wand, fuzz );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::paintOpaqueImageChannel( CHANNELTYPE channel, ImagickPixel target, ImagickPixel fill, float fuzz)
	Changes any pixel that matches color with the color defined by fill.
*/
PHP_METHOD(imagick, paintopaqueimagechannel)
{
	php_imagick_object *intern;
	php_imagickpixel_object *intern_fill, *intern_target;
	zval *object, *targetObj, *fillObj;
	long channel;
	double fuzz;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "lOOd", &channel, &targetObj, php_imagickpixel_sc_entry, &fillObj, php_imagickpixel_sc_entry, &fuzz ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_target = (php_imagickpixel_object *)zend_object_store_get_object(targetObj TSRMLS_CC);
	intern_fill = (php_imagickpixel_object *)zend_object_store_get_object(fillObj TSRMLS_CC);

	status = MagickPaintOpaqueImageChannel( intern->magick_wand, channel, intern_target->pixel_wand, intern_fill->pixel_wand, fuzz );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::optimizeImageLayers()
	Compares each image the GIF disposed forms of the previous image in the sequence.  From this it attempts to select the smallest cropped image to replace each frame, while preserving the results of the animation.
*/
PHP_METHOD(imagick, optimizeimagelayers)
{
	MagickWand *tmpWand;
	zval *object;
	php_imagick_object *intern, *intern_return;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = (MagickWand *)MagickOptimizeImageLayers( intern->magick_wand );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Optimize image layers failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	return;
}
/* }}} */

/* {{{ proto Imagick::paintTransparentImage( ImagickPixel target, float alpha, float fuzz)
	Changes any pixel that matches color with the color defined by fill.
*/
PHP_METHOD(imagick, painttransparentimage)
{
	php_imagick_object *intern;
	php_imagickpixel_object *intern_target;
	zval *object, *targetObj;
	double alpha, fuzz;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "Odd", &targetObj, php_imagickpixel_sc_entry, &alpha, &fuzz ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_target = (php_imagickpixel_object *)zend_object_store_get_object(targetObj TSRMLS_CC);
	status = MagickPaintTransparentImage( intern->magick_wand, intern_target->pixel_wand, alpha, fuzz );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::paintTransparentImage( PREVIEWTYPE preview )
	Tiles 9 thumbnails of the specified image with an image processing operation applied at varying strengths.  This is helpful to quickly pin-point an appropriate parameter for an image processing operation.
*/
PHP_METHOD(imagick, previewimages)
{
	php_imagick_object *intern, *intern_return;
	zval *object;
	long preview;
	MagickWand *tmpWand;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &preview ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickPreviewImages( intern->magick_wand, preview );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Preview image failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;
	return;
}
/* }}} */

/* {{{ proto Imagick::profileImage( string name,string profile)
	Adds or removes a ICC, IPTC, or generic profile from an image.  If the profile is NULL, it is removed from the image otherwise added.  Use a name of '*' and a profile of NULL to remove all profiles from the image.
*/
PHP_METHOD(imagick, profileimage)
{
	php_imagick_object *intern;
	zval *object;
	char *name, *profile;
	int nameLen;
	size_t profileLen;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &nameLen, &profile, &profileLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickProfileImage( intern->magick_wand, name, profile, profileLen );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto Imagick::quantizeImage( int numberColors, COLORSPACE colorspace, int treedepth, bool dither,
    bool measureError)
	Analyzes the colors within a reference image and chooses a fixed number of colors to represent the image.  The goal of the algorithm is to minimize the color difference between the input and output image while minimizing the processing time.
*/
PHP_METHOD(imagick, quantizeimage)
{
	php_imagick_object *intern;
	zval *object;
	long numberColors, colorSpaceType, treeDepth;
	zend_bool dither, measureError;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 5 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "lllbb", &numberColors, &colorSpaceType, &treeDepth, &dither, &measureError ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickQuantizeImage(intern->magick_wand, numberColors, colorSpaceType, treeDepth, dither, measureError );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

PHP_METHOD(imagick, quantizeimages)
{
	php_imagick_object *intern;
	zval *object;
	long numberColors, colorSpaceType, treeDepth;
	zend_bool dither, measureError;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 5 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "lllbb", &numberColors, &colorSpaceType, &treeDepth, &dither, &measureError ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickQuantizeImages(intern->magick_wand, numberColors, colorSpaceType, treeDepth, dither, measureError );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, reducenoiseimage)
{
	php_imagick_object *intern;
	zval *object;
	double radius;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &radius ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickReduceNoiseImage( intern->magick_wand, radius );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, removeimageprofile)
{
	php_imagick_object *intern;
	zval *object;
	char *name;
	unsigned char *profile;
	int nameLen;
#if MagickLibVersion < 0x628
	long profileLen;
#else
	size_t profileLen;
#endif


	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &nameLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	profile = MagickRemoveImageProfile( intern->magick_wand, name, &profileLen );

	ZVAL_STRING( return_value, (char *)profile, 1 );
	IMAGICK_FREE_MEMORY( unsigned char *, profile );
	return;
}

PHP_METHOD(imagick, separateimagechannel)
{
	php_imagick_object *intern;
	zval *object;
	long channelType;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &channelType ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSeparateImageChannel( intern->magick_wand, channelType);

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, sepiatoneimage)
{
	php_imagick_object *intern;
	zval *object;
	double threshold;
	double c_opacity;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &threshold ) == FAILURE )
	{
		return;
	}

	c_opacity = ( threshold * QuantumRange ) / 100;

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSepiaToneImage( intern->magick_wand, c_opacity );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setimagebias)
{
	php_imagick_object *intern;
	zval *object;
	double bias;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &bias ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageBias( intern->magick_wand, bias );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setimageblueprimary)
{
	php_imagick_object *intern;
	zval *object;
	double x, y;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageBluePrimary( intern->magick_wand, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setimagebordercolor)
{
	zval *object, *objvar;
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &objvar, php_imagickpixel_sc_entry) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	internp = (php_imagickpixel_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	status = MagickSetImageBorderColor( intern->magick_wand, internp->pixel_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setimagechanneldepth)
{
	php_imagick_object *intern;
	zval *object;
	long channelType, depth;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ll", &channelType, &depth ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageChannelDepth( intern->magick_wand, channelType, depth );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setimagecolormapcolor)
{
}

PHP_METHOD(imagick, setimagecolorspace)
{
	php_imagick_object *intern;
	zval *object;
	long colorSpace;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &colorSpace ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageColorspace( intern->magick_wand, colorSpace );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setimagedispose)
{
	php_imagick_object *intern;
	zval *object;
	long composite;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &composite ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageCompose( intern->magick_wand, composite );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setimageextent)
{
	php_imagick_object *intern;
	zval *object;
	long rows, columns;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ll", &rows, &columns ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageExtent( intern->magick_wand, rows, columns );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setimagegreenprimary)
{
	php_imagick_object *intern;
	zval *object;
	double x, y;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageGreenPrimary( intern->magick_wand, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setimageinterlacescheme) // TODO FIX THIS!
{
	php_imagick_object *intern;
	zval *object;
	long interlace;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &interlace ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageInterlaceScheme( intern->magick_wand, interlace );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setimageinterpolatemethod) // TODO FIX THIS!
{
	php_imagick_object *intern;
	zval *object;
	long interpolate;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &interpolate ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageInterpolateMethod( intern->magick_wand, interpolate );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;

}

PHP_METHOD(imagick, setimageprofile) // TODO FIX THIS!
{
	php_imagick_object *intern;
	zval *object;
	char *name, *profile;
	size_t profileLen;
	int nameLen;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &nameLen, &profile, &profileLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageProfile( intern->magick_wand, name, profile, profileLen );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setimageredprimary)
{
	php_imagick_object *intern;
	zval *object;
	double x, y;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageRedPrimary( intern->magick_wand, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setimagerenderingintent)
{
	php_imagick_object *intern;
	zval *object;
	long renderingIntent;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &renderingIntent ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageRenderingIntent( intern->magick_wand, renderingIntent );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setimagevirtualpixelmethod)
{
	php_imagick_object *intern;
	zval *object;
	long virtualPixel;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &virtualPixel ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageVirtualPixelMethod( intern->magick_wand, virtualPixel );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setimagewhitepoint)
{
	php_imagick_object *intern;
	zval *object;
	double x, y;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageWhitePoint( intern->magick_wand, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, sigmoidalcontrastimage)
{
	php_imagick_object *intern;
	zval *object;
	zend_bool sharpen;
	double alpha, beta;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "bdd", &sharpen, &alpha, &beta ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSigmoidalContrastImage( intern->magick_wand, sharpen, alpha, beta );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, sigmoidalcontrastimagechannel)
{
	php_imagick_object *intern;
	zval *object;
	long channel;
	zend_bool sharpen;
	double alpha, beta;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "lbdd", &channel, &sharpen, &alpha, &beta ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSigmoidalContrastImageChannel( intern->magick_wand, channel, sharpen, alpha, beta );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, stereoimage)
{
	MagickWand *tmpWand;
	zval *object, *magickObject;
	php_imagick_object *intern, *intern_second, *intern_return;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "O", &magickObject, php_imagick_sc_entry ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_second = (php_imagick_object *)zend_object_store_get_object(magickObject TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern_second->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickStereoImage( intern->magick_wand, intern_second->magick_wand );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Stereo image failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	return;
}

PHP_METHOD(imagick, textureimage)
{
	MagickWand *tmpWand;
	zval *object, *magickObject;
	php_imagick_object *intern, *intern_second, *intern_return;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "O", &magickObject, php_imagick_sc_entry ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_second = (php_imagick_object *)zend_object_store_get_object(magickObject TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern_second->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickTextureImage( intern->magick_wand, intern_second->magick_wand );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Texture image failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	return;
}

PHP_METHOD(imagick, tintimage)
{
	php_imagick_object *intern;
	php_imagickpixel_object *intern_tint, *intern_opacity;
	zval *object, *tintObj, *opacityObj;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "OO", &tintObj, php_imagickpixel_sc_entry, &opacityObj, php_imagickpixel_sc_entry ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_tint = (php_imagickpixel_object *)zend_object_store_get_object(tintObj TSRMLS_CC);
	intern_opacity = (php_imagickpixel_object *)zend_object_store_get_object(opacityObj TSRMLS_CC);

	status = MagickTintImage( intern->magick_wand, intern_tint->pixel_wand, intern_opacity->pixel_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, uniqueimagecolors)
{
	zval *object;
	php_imagick_object *intern;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickUniqueImageColors( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, unsharpmaskimage)
{
	php_imagick_object *intern;
	zval *object;
	double radius, sigma, amount, threshold;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddd", &radius, &sigma, &amount, &threshold ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickUnsharpMaskImage( intern->magick_wand, radius, sigma, amount, threshold );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, unsharpmaskimagechannel)
{
	php_imagick_object *intern;
	zval *object;
	long channel;
	double radius, sigma, amount, threshold;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 5 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ldddd", &channel, &radius, &sigma, &amount, &threshold ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickUnsharpMaskImageChannel( intern->magick_wand, channel, radius, sigma, amount, threshold );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, contraststretchimage)
{

	php_imagick_object *intern;
	zval *object;
	double blackPoint, whitePoint;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &blackPoint, &whitePoint ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickContrastStretchImage( intern->magick_wand, blackPoint, whitePoint );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, contraststretchimagechannel)
{
	php_imagick_object *intern;
	zval *object;
	long channel;
	double blackPoint, whitePoint;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ldd", &channel, &blackPoint, &whitePoint ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickContrastStretchImageChannel( intern->magick_wand, channel, blackPoint, whitePoint );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, convolveimage)
{
	php_imagick_object *intern;
	zval *object;
	long order;
	MagickBooleanType status;
	zval *kernelArray;
	double *kernel;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "a",  &kernelArray ) == FAILURE )
	{
		return;
	}

	kernel = getDoublesFromZval( kernelArray, &order TSRMLS_CC );

	if( kernel == (double *)NULL )
	{
		throwExceptionWithMessage( 1, "Unable to read matrix array.", 1 TSRMLS_CC );
		RETURN_FALSE;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickConvolveImage( intern->magick_wand, order, kernel );
	efree( kernel );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, convolveimagechannel)
{
	php_imagick_object *intern;
	zval *object;
	long order, channel;
	MagickBooleanType status;
	zval *kernelArray;
	double *kernel;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "la", &channel, &kernelArray ) == FAILURE )
	{
		return;
	}

	kernel = getDoublesFromZval( kernelArray, &order TSRMLS_CC );

	if( kernel == (double *)NULL )
	{
		throwExceptionWithMessage( 1, "Unable to read matrix array.", 1 TSRMLS_CC );
		RETURN_FALSE;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickConvolveImageChannel( intern->magick_wand, channel, order, kernel );
	efree( kernel );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, cyclecolormapimage)
{
	php_imagick_object *intern;
	zval *object;
	long displace;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &displace ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickCycleColormapImage( intern->magick_wand, displace );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, deconstructimages)
{
	MagickWand *tmpWand;
	zval *object;
	php_imagick_object *intern, *intern_return;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickDeconstructImages( intern->magick_wand );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Deconstruct image failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	return;
}

PHP_METHOD(imagick, getimageregion)
{
	MagickWand *tmpWand;
	zval *object;
	php_imagick_object *intern, *intern_return;
	long width, height, x, y;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "llll", &width, &height, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickGetImageRegion( intern->magick_wand, width, height, x, y );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Get image region failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	return;

}
PHP_METHOD(imagick, despeckleimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickDespeckleImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, edgeimage)
{
	php_imagick_object *intern;
	zval *object;
	double radius;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &radius ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickEdgeImage( intern->magick_wand, radius );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;

}

PHP_METHOD(imagick, embossimage)
{
	php_imagick_object *intern;
	zval *object;
	double radius, sigma;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &radius, &sigma ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickEmbossImage( intern->magick_wand, radius, sigma );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, enhanceimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickEnhanceImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, equalizeimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickEqualizeImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, evaluateimage)
{
	php_imagick_object *intern;
	zval *object;
	long evaluateOperator;
	double constant;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ld", &evaluateOperator, &constant ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickEvaluateImage( intern->magick_wand, evaluateOperator, constant );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, evaluateimagechannel)
{
	php_imagick_object *intern;
	zval *object;
	long channel, evaluateOperator;
	double constant;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "lld", &channel, &evaluateOperator, &constant ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickEvaluateImageChannel( intern->magick_wand, channel, evaluateOperator, constant );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, getimagegeometry)
{

	long width,height;
	php_imagick_object *intern;
	zval *object;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	width = MagickGetImageWidth( intern->magick_wand );
	height = MagickGetImageHeight( intern->magick_wand );

	array_init( return_value );
	add_assoc_long( return_value, "width", width );
	add_assoc_long( return_value, "height", height );

	return;
}

PHP_METHOD(imagick, getimagebackgroundcolor)
{
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	zval *object;
	MagickBooleanType status;
	PixelWand *tmpWand;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagickpixel_sc_entry );

	tmpWand = NewPixelWand();
	status = MagickGetImageBackgroundColor( intern->magick_wand, tmpWand );

	if ( tmpWand == (PixelWand *)NULL || !IsPixelWand( tmpWand ) )
	{
		throwExceptionWithMessage( 4, "Get image backgroundcolor failed", 4 TSRMLS_CC );
		RETURN_FALSE;
	}

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	internp = (php_imagickpixel_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	internp->pixel_wand = tmpWand;

	return;
}

PHP_METHOD(imagick, getimageblueprimary)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	double x, y;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickGetImageBluePrimary( intern->magick_wand, &x, &y );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	array_init( return_value );
	add_assoc_double( return_value, "x", x );
	add_assoc_double( return_value, "y", y );

	return;
}

PHP_METHOD(imagick, getimagebordercolor)
{
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	zval *object;
	MagickBooleanType status;
	PixelWand *tmpWand;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagickpixel_sc_entry );

	tmpWand = NewPixelWand();
	status = MagickGetImageBorderColor( intern->magick_wand, tmpWand );

	if ( tmpWand == (PixelWand *)NULL || !IsPixelWand( tmpWand ) )
	{
		throwExceptionWithMessage( 4, "Get image backgroundcolor failed", 4 TSRMLS_CC );
		RETURN_FALSE;
	}

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	internp = (php_imagickpixel_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	internp->pixel_wand = tmpWand;

	return;
}

PHP_METHOD(imagick, getimagechanneldepth)
{
	php_imagick_object *intern;
	long channelType, channelDepth;
	zval *object;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &channelType ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	channelDepth = MagickGetImageChannelDepth( intern->magick_wand, channelType );
	RETVAL_LONG( channelDepth );
}

PHP_METHOD(imagick, getimagechanneldistortion)
{
	zval *object, *objvar;
	php_imagick_object *intern, *intern_second;
	long channelType, metricType;
	double distortion;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Oll", &objvar, php_imagick_sc_entry, &channelType, &metricType ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_second = (php_imagick_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern_second->magick_wand, 1, 1 );

	status = MagickGetImageChannelDistortion( intern->magick_wand, intern_second->magick_wand, channelType, metricType, &distortion );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETVAL_DOUBLE( distortion );

}

PHP_METHOD(imagick, getimagechannelextrema)
{
	zval *object;
	php_imagick_object *intern;
	long channelType;
	unsigned long minima, maxima;
	MagickBooleanType status;


	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &channelType ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickGetImageChannelExtrema( intern->magick_wand, channelType, &minima, &maxima );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	array_init( return_value );
	add_assoc_long( return_value, "minima", minima );
	add_assoc_long( return_value, "maxima", maxima );

	return;

}

PHP_METHOD(imagick, getimagechannelmean)
{
	zval *object;
	php_imagick_object *intern;
	long channelType;
	double mean, standardDeviation;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &channelType ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickGetImageChannelMean( intern->magick_wand, channelType, &mean, &standardDeviation );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	array_init( return_value );
	add_assoc_long( return_value, "mean", mean );
	add_assoc_long( return_value, "standardDeviation", standardDeviation );

	return;
}


PHP_METHOD(imagick, getimagechannelstatistics)
{
	zval *object, *tmp;

	const long channels[] = { UndefinedChannel, RedChannel, CyanChannel,
							  GreenChannel, MagentaChannel, BlueChannel,
							  YellowChannel, OpacityChannel, BlackChannel,
							  MatteChannel };
	php_imagick_object *intern;
	ChannelStatistics *statistics;
	int elements = 10, i;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	statistics = MagickGetImageChannelStatistics( intern->magick_wand );
	array_init( return_value );

	for ( i = 0; i < elements ; i++ )
	{
		MAKE_STD_ZVAL( tmp );
		array_init( tmp );

		add_assoc_double( tmp, "mean", statistics[channels[i]].mean );
		add_assoc_double( tmp, "minima", statistics[channels[i]].minima );
		add_assoc_double( tmp, "maxima", statistics[channels[i]].maxima );
		add_assoc_double( tmp, "standardDeviation", statistics[channels[i]].standard_deviation );
		add_assoc_long( tmp, "scale", statistics[channels[i]].scale );
		add_assoc_long( tmp, "depth", statistics[channels[i]].depth );
		add_index_zval( return_value, channels[i], tmp );
	}
	return;
}

PHP_METHOD(imagick, getimagecolormapcolor)
{
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	zval *object;
	MagickBooleanType status;
	PixelWand *tmpWand;
	long index;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagickpixel_sc_entry );

	tmpWand = NewPixelWand();
	status = MagickGetImageColormapColor( intern->magick_wand, index , tmpWand );

	if ( tmpWand == (PixelWand *)NULL || !IsPixelWand( tmpWand ) )
	{
		throwExceptionWithMessage( 4, "Get image colormap color failed", 4 TSRMLS_CC );
		RETURN_FALSE;
	}

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	internp = (php_imagickpixel_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	internp->pixel_wand = tmpWand;

	return;
}

PHP_METHOD(imagick, getimagecolorspace)
{
	php_imagick_object *intern;
	zval *object;
	long colorSpace;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	colorSpace = MagickGetImageColorspace( intern->magick_wand );
	RETVAL_LONG( colorSpace );
}

PHP_METHOD(imagick, getimagecompose)
{
	php_imagick_object *intern;
	zval *object;
	long composite;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	composite = MagickGetImageCompose( intern->magick_wand );
	RETVAL_LONG( composite );
}

PHP_METHOD(imagick, getimagedelay)
{
	php_imagick_object *intern;
	zval *object;
	long delay;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	delay = MagickGetImageDelay( intern->magick_wand );
	RETVAL_LONG( delay );
}

PHP_METHOD(imagick, getimagedepth)
{
	php_imagick_object *intern;
	zval *object;
	long depth;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	depth = MagickGetImageDepth( intern->magick_wand );
	RETVAL_LONG( depth );
}

PHP_METHOD(imagick, getimagedistortion)
{
	zval *object, *objvar;
	php_imagick_object *intern, *intern_second;
	long metricType;
	double distortion;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ol", &objvar, php_imagick_sc_entry, &metricType ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_second = (php_imagick_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern_second->magick_wand, 1, 1 );

	status = MagickGetImageDistortion( intern->magick_wand, intern_second->magick_wand, metricType, &distortion );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETVAL_DOUBLE( distortion );
}

PHP_METHOD(imagick, getimageextrema)
{
	zval *object;
	php_imagick_object *intern;
	unsigned long min, max;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickGetImageExtrema( intern->magick_wand, &min, &max );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	array_init( return_value );
	add_assoc_long( return_value, "min", min );
	add_assoc_long( return_value, "max", max );

	return;
}



PHP_METHOD(imagick, getimagedispose)
{
	zval *object;
	php_imagick_object *intern;
	long dispose;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	dispose = MagickGetImageDispose( intern->magick_wand );
	RETVAL_LONG( dispose );
}

PHP_METHOD(imagick, getimagegamma)
{
	zval *object;
	php_imagick_object *intern;
	double gamma;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	gamma = MagickGetImageGamma( intern->magick_wand );
	RETVAL_DOUBLE( gamma );
}

PHP_METHOD(imagick, getimagegreenprimary)
{
	php_imagick_object *intern;
	zval *object;
	double x, y;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickGetImageGreenPrimary( intern->magick_wand, &x, &y );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	array_init( return_value );
	add_assoc_double( return_value, "x", x );
	add_assoc_double( return_value, "y", y );

	return;
}

PHP_METHOD(imagick, getimageheight)
{
	php_imagick_object *intern;
	zval *object;
	long height;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	height = MagickGetImageHeight( intern->magick_wand );
	RETVAL_LONG( height );
}

PHP_METHOD(imagick, getimagehistogram) // TODO: does this work like iterator?
{
}

PHP_METHOD(imagick, getimageinterlacescheme)
{
	php_imagick_object *intern;
	zval *object;
	long interlace;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	interlace = MagickGetImageInterlaceScheme( intern->magick_wand );
	RETVAL_LONG( interlace );
}

PHP_METHOD(imagick, getimageinterpolatemethod)
{
	php_imagick_object *intern;
	zval *object;
	long interpolate;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	interpolate = MagickGetImageInterpolateMethod( intern->magick_wand );
	RETVAL_LONG( interpolate );
}

PHP_METHOD(imagick, getimageiterations)
{
	php_imagick_object *intern;
	zval *object;
	long iterations;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	iterations = MagickGetImageIterations( intern->magick_wand );
	RETVAL_LONG( iterations );
}

PHP_METHOD(imagick, getimagematte)
{
	php_imagick_object *intern;
	zval *object;
	long matte;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	matte = MagickGetImageMatte( intern->magick_wand );
	RETVAL_LONG( matte );
}

PHP_METHOD(imagick, getimagemattecolor)
{
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	zval *object;
	MagickBooleanType status;
	PixelWand *tmpWand;


	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagickpixel_sc_entry );

	tmpWand = NewPixelWand();
	status = MagickGetImageMatteColor( intern->magick_wand, tmpWand );

	if ( tmpWand == (PixelWand *)NULL || !IsPixelWand( tmpWand ) )
	{
		throwExceptionWithMessage( 4, "Get image matte color failed", 4 TSRMLS_CC );
		RETURN_FALSE;
	}

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	internp = (php_imagickpixel_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	internp->pixel_wand = tmpWand;

	return;
}

PHP_METHOD(imagick, getimagepage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	unsigned long width, height;
	long x, y;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickGetImagePage( intern->magick_wand, &width, &height, &x, &y );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	array_init( return_value );

	add_assoc_long( return_value, "width", width );
	add_assoc_long( return_value, "height", height );
	add_assoc_long( return_value, "x", x );
	add_assoc_long( return_value, "y", y );

	return;
}

PHP_METHOD(imagick, getimagepixelcolor)
{
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	zval *object;
	MagickBooleanType status;
	PixelWand *tmpWand;
	long x, y;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &x, &y ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagickpixel_sc_entry );

	tmpWand = NewPixelWand();
	status = MagickGetImagePixelColor( intern->magick_wand, x, y , tmpWand );

	if ( tmpWand == (PixelWand *)NULL || !IsPixelWand( tmpWand ) )
	{
		throwExceptionWithMessage( 4, "Get image pixel color failed", 4 TSRMLS_CC );
		RETURN_FALSE;
	}

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	internp = (php_imagickpixel_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	internp->pixel_wand = tmpWand;

	return;
}

PHP_METHOD(imagick, getimageprofile)
{
	php_imagick_object *intern;
	zval *object;
	char *profile, *name;
	int nameLen;
#if MagickLibVersion < 0x628
	long length;
#else
	size_t length;
#endif

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &nameLen ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	profile = (char *)MagickGetImageProfile( intern->magick_wand, name, &length );

	ZVAL_STRING( return_value, profile, 1 );
	IMAGICK_FREE_MEMORY( char *, profile );
	return;
}

PHP_METHOD(imagick, getimageredprimary)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	double x, y;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickGetImageRedPrimary( intern->magick_wand, &x, &y );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	array_init( return_value );
	add_assoc_double( return_value, "x", x );
	add_assoc_double( return_value, "y", y );

	return;
}

PHP_METHOD(imagick, getimagerenderingintent)
{
	php_imagick_object *intern;
	zval *object;
	long renderingIntent;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	renderingIntent = MagickGetImageRenderingIntent( intern->magick_wand );
	RETVAL_LONG( renderingIntent );
}

PHP_METHOD(imagick, getimageresolution)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	double x, y;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickGetImageResolution( intern->magick_wand, &x, &y );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	array_init( return_value );
	add_assoc_double( return_value, "x", x );
	add_assoc_double( return_value, "y", y );

	return;
}

PHP_METHOD(imagick, getimagescene)
{
	php_imagick_object *intern;
	zval *object;
	unsigned long scene;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	scene = MagickGetImageScene( intern->magick_wand );
	RETVAL_LONG( scene );
}

PHP_METHOD(imagick, getimagesignature)
{
	php_imagick_object *intern;
	zval *object;
	char *signature;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	signature = MagickGetImageSignature( intern->magick_wand );
	ZVAL_STRING( return_value, signature, 1 );
	IMAGICK_FREE_MEMORY( char *, signature );
	return;
}

PHP_METHOD(imagick, getimagetickspersecond)
{
	php_imagick_object *intern;
	zval *object;
	unsigned long ticks;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	ticks = MagickGetImageTicksPerSecond( intern->magick_wand );
	RETVAL_LONG( ticks );
}

PHP_METHOD(imagick, getimagetype)
{
	php_imagick_object *intern;
	zval *object;
	long imageType;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	imageType = MagickGetImageType( intern->magick_wand );
	RETVAL_LONG( imageType );
}

PHP_METHOD(imagick, getimageunits)
{
	php_imagick_object *intern;
	zval *object;
	long resolutionType;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	resolutionType = MagickGetImageType( intern->magick_wand );
	RETVAL_LONG( resolutionType );
}

PHP_METHOD(imagick, getimagevirtualpixelmethod)
{
	php_imagick_object *intern;
	zval *object;
	long pixelMethod;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	pixelMethod = MagickGetImageVirtualPixelMethod( intern->magick_wand );
	RETVAL_LONG( pixelMethod );
}

PHP_METHOD(imagick, getimagewhitepoint)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	double x, y;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickGetImageWhitePoint( intern->magick_wand, &x, &y );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	array_init( return_value );
	add_assoc_double( return_value, "x", x );
	add_assoc_double( return_value, "y", y );

	return;
}

PHP_METHOD(imagick, getimagewidth)
{
	php_imagick_object *intern;
	zval *object;
	unsigned long width;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	width = MagickGetImageWidth( intern->magick_wand );
	RETVAL_LONG( width );
}

PHP_METHOD(imagick, getnumberimages)
{
	php_imagick_object *intern;
	zval *object;
	unsigned long numImages;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	numImages = MagickGetNumberImages( intern->magick_wand );
	RETVAL_LONG( numImages );
}

PHP_METHOD(imagick, thumbnailimage)
{
	long x, y;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	long imageY, imageX;
	double tmp;



	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ll", &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );


	if ( ( x == 0 ) && ( y == 0 ) )
	{
		throwExceptionWithMessage( 1, "Can't thumbnail image to zero size.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	if ( ( x == 0 ) || ( y == 0 ) )
	{
		imageX = MagickGetImageWidth( intern->magick_wand );
		imageY = MagickGetImageHeight( intern->magick_wand );

		if( x == 0 )
		{
			tmp = (double)imageX / (double)imageY;
			x = tmp * (double)y;
		}

		if ( y == 0 )
		{
			tmp = (double)imageY / (double)imageX;
			y = tmp * (double)x;
		}
	}

	status = MagickThumbnailImage( intern->magick_wand, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, resetiterator)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	status = IsMagickWand( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}
	intern->next_out_of_bound = 0;
	MagickResetIterator( intern->magick_wand );
	RETURN_TRUE;
}


PHP_METHOD(imagick, setfirstiterator)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	status = IsMagickWand( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}
	intern->next_out_of_bound = 0;
	MagickSetFirstIterator( intern->magick_wand );
	RETURN_TRUE;
}


PHP_METHOD(imagick, previousimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	status = MagickPreviousImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}
	intern->next_out_of_bound = 0;
	RETURN_TRUE;
}


PHP_METHOD(imagick, nextimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickNextImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		intern->next_out_of_bound = 1;
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, haspreviousimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	status = MagickHasPreviousImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, hasnextimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	status = MagickHasNextImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, getimageindex)
{
	zval *object;
	MagickBooleanType status;
	php_imagick_object *intern;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	status = MagickGetImageIndex( intern->magick_wand );
	ZVAL_LONG( return_value, (long)status );
	return;
}


PHP_METHOD(imagick, setimageindex)
{
	const long index;
	zval *object;
	MagickBooleanType status;
	php_imagick_object *intern;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &index ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickSetImageIndex( intern->magick_wand, index );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	intern->next_out_of_bound = 0;
	RETURN_TRUE;

}


PHP_METHOD(imagick, removeimage)
{
	zval *object;
	MagickBooleanType status;
	php_imagick_object *intern;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickRemoveImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	intern->next_out_of_bound = 0;
	RETURN_TRUE;
}


PHP_METHOD(imagick, getimagefilename)
{
	php_imagick_object *intern;
	zval *object;
	char *fileName;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	fileName = MagickGetImageFilename( intern->magick_wand );
	ZVAL_STRING( return_value, fileName, 1 );
	IMAGICK_FREE_MEMORY( char *, fileName );
	return;
}


PHP_METHOD(imagick, getimagesize)
{
	php_imagick_object *intern;
	zval *object;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	ZVAL_LONG( return_value, (long)MagickGetImageSize( intern->magick_wand ) );
	return;
}


PHP_METHOD(imagick, getimageblob)
{
	php_imagick_object *intern;
	zval *object;
	unsigned char *imageContents;
	size_t imageSize;
	char *buffer;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	IMAGICK_HAS_FORMAT( buffer, intern->magick_wand );

	imageContents = MagickGetImageBlob( intern->magick_wand, &imageSize );
	ZVAL_STRINGL( return_value, (char *)imageContents, imageSize, 1 );
	IMAGICK_FREE_MEMORY( unsigned char *, imageContents );
	return;
}


PHP_METHOD(imagick, getimageformat)
{
	php_imagick_object *intern;
	zval *object;
	char *format;
	char *buffer;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	IMAGICK_HAS_FORMAT( buffer, intern->magick_wand );

	format = MagickGetImageFormat( intern->magick_wand );
	ZVAL_STRING( return_value, format, 1 );
	IMAGICK_FREE_MEMORY( char *, format );
	return;
}


/* FIX THIS WHOLE FUNCTION */
PHP_METHOD(imagick, identifyimage)
{
	php_imagick_object *intern;
	char *identity, *hashValue;
	HashTable *hash_table;
	zend_bool appendRawString = 0;
	zval  *delim, *zident, *object, *array;
	long newLines, i, elements;

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "|b", &appendRawString ) == FAILURE )
	{
		return;
	}

	identity = MagickIdentifyImage( intern->magick_wand );

	/* Explode on newlines */
	MAKE_STD_ZVAL( delim );
	ZVAL_STRING( delim, "\n", 0 );

	/* Create zval from identity */
	MAKE_STD_ZVAL( zident );
	ZVAL_STRING( zident, identity, 0 );

	/* Initialize empty array */
	MAKE_STD_ZVAL( array );
	array_init( array );

	/* count occurances and get that amount of elements */
	newLines = count_occurences_of( '\n', identity TSRMLS_CC);
	php_explode( delim, zident, array, newLines);

	/* free useless values */
	FREE_ZVAL( zident );
	FREE_ZVAL( delim );

	/* initialize return value and get hash table */
	array_init( return_value );
	hash_table = Z_ARRVAL_P( array );

	elements = zend_hash_num_elements( hash_table );

	if ( elements == 0 )
	{
		zval_dtor( array );
		FREE_ZVAL( array );
		throwExceptionWithMessage( 1, "Identifying image failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	zend_hash_internal_pointer_reset_ex( hash_table, (HashPosition *) 0 );

	for ( i = 0 ; i < elements ; i++ )
	{

		hashValue = getHashValue( hash_table TSRMLS_CC);

		if ( strlen( hashValue ) < 3 )
		{
			continue;
		}

		/* It would be pain in the ass ( yes, in my ass. ) to parse all the values */
		add_assoc_string_helper( return_value, "Image: ", "imageName", hashValue TSRMLS_CC);
		add_assoc_string_helper( return_value, "Format: ", "format", hashValue TSRMLS_CC);
		add_assoc_string_helper( return_value, "Geometry: ", "geometry", hashValue TSRMLS_CC);
		add_assoc_string_helper( return_value, "Units: ", "units", hashValue TSRMLS_CC);
		add_assoc_string_helper( return_value, "Type: ", "type", hashValue TSRMLS_CC);
		add_assoc_string_helper( return_value, "Resolution: ", "resolution", hashValue TSRMLS_CC);
		add_assoc_string_helper( return_value, "Colorspace: ", "colorSpace", hashValue TSRMLS_CC);
		add_assoc_string_helper( return_value, "Filesize: ", "fileSize", hashValue TSRMLS_CC);
		add_assoc_string_helper( return_value, "Compression: ", "compression", hashValue TSRMLS_CC);
		add_assoc_string_helper( return_value, "Signature: ", "signature", hashValue TSRMLS_CC);

		efree( hashValue );
	}

	/* if user wants raw string append it */
	if ( appendRawString == 1 )
	{
		add_assoc_string( return_value, "rawOutput", identity, 1 );
	}

	zval_dtor( array );			 /* let it fly free.. */
	FREE_ZVAL( array );

	IMAGICK_FREE_MEMORY( char *, identity );
	return;
}


PHP_METHOD(imagick, getimagecolors)
{
	php_imagick_object *intern;
	zval *object;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	ZVAL_LONG( return_value, (long)MagickGetImageColors( intern->magick_wand ) );
	return;
}


PHP_METHOD(imagick, commentimage)
{
	php_imagick_object *intern;

	char *comment;
	int commentLen;

	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &comment, &commentLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickCommentImage( intern->magick_wand, comment );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, setimagefilename)
{
	php_imagick_object *intern;

	char *fileName;
	int fileNameLen;

	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &fileName, &fileNameLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageFilename( intern->magick_wand, fileName );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, setimageattribute)
{
	php_imagick_object *intern;

	char *key, *attribute;
	int keyLen, attributeLen;

	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ss", &key, &keyLen, &attribute, &attributeLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageAttribute( intern->magick_wand, key, attribute );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setimagebackgroundcolor)
{
	zval *object;
	zval *objvar;
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &objvar, php_imagickpixel_sc_entry) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	internp = (php_imagickpixel_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	status = MagickSetImageBackgroundColor( intern->magick_wand, internp->pixel_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}



PHP_METHOD(imagick, setimagecompose)
{
	php_imagick_object *intern;
	long compose;

	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &compose ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageCompose( intern->magick_wand, compose );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setimagecompression)
{
	php_imagick_object *intern;
	long compression;

	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &compression ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageCompression( intern->magick_wand, compression );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setimagedelay)
{
	php_imagick_object *intern;
	long delay;

	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &delay ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageDelay( intern->magick_wand, delay );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, colorizeimage)
{
	php_imagick_object *intern;
	php_imagickpixel_object *intern_color, *intern_opacity;
	zval *object, *colorObj, *opacityObj;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "OO", &colorObj, php_imagickpixel_sc_entry, &opacityObj, php_imagickpixel_sc_entry ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_color = (php_imagickpixel_object *)zend_object_store_get_object(colorObj TSRMLS_CC);
	intern_opacity = (php_imagickpixel_object *)zend_object_store_get_object(opacityObj TSRMLS_CC);

	status = MagickColorizeImage( intern->magick_wand, intern_color->pixel_wand, intern_opacity->pixel_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;

}

PHP_METHOD(imagick, compareimagechannels)
{
	MagickWand *tmpWand;
	zval *object, *objvar;
	php_imagick_object *intern, *intern_second, *intern_return;
	long channelType, metricType;
	double distortion;
	zval *newWand;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Oll", &objvar, php_imagick_sc_entry, &channelType, &metricType ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_second = (php_imagick_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern_second->magick_wand, 1, 1 );

	MAKE_STD_ZVAL( newWand );
	array_init( return_value );

	object_init_ex( newWand, php_imagick_sc_entry );
	tmpWand = MagickCompareImageChannels( intern->magick_wand, intern_second->magick_wand, channelType, metricType, &distortion );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Compare image channels failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(newWand TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	add_next_index_zval( return_value, newWand );
	add_next_index_double( return_value, distortion );

	return;
}

PHP_METHOD(imagick, compareimagelayers)
{
	MagickWand *tmpWand;
	zval *object;
	php_imagick_object *intern, *intern_return;
	long compareMethod;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &compareMethod ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = (MagickWand *)MagickCompareImageLayers( intern->magick_wand, compareMethod );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Compare image layers failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	return;

}

PHP_METHOD(imagick, flattenimages)
{
	php_imagick_object *intern, *intern_return;
	zval *object;
	MagickWand *tmpWand;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickFlattenImages( intern->magick_wand );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Flatten images failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	RETURN_TRUE;
}

PHP_METHOD(imagick, flipimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickFlipImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, flopimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickFlopImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, frameimage)
{
	zval *object, *objvar;
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	MagickBooleanType status;
	long width, height, innerBevel, outerBevel;

	if ( ZEND_NUM_ARGS() != 5 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ollll", &objvar, php_imagickpixel_sc_entry, &width, &height, &innerBevel, &outerBevel ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	internp = (php_imagickpixel_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	status = MagickFrameImage( intern->magick_wand, internp->pixel_wand, width, height, innerBevel, outerBevel );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, fximage)
{
	MagickWand *tmpWand;
	php_imagick_object *intern, *intern_return;
	zval *object;
	char *expression;
	int expressionLen;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &expression, &expressionLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickFxImage( intern->magick_wand, expression );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Fx image failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	RETURN_TRUE;
}

PHP_METHOD(imagick, fximagechannel)
{
	MagickWand *tmpWand;
	php_imagick_object *intern, *intern_return;
	zval *object;
	char *expression;
	int expressionLen;
	long channel;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ls", &channel, &expression, &expressionLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickFxImageChannel( intern->magick_wand, channel, expression );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Fx image channel failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	RETURN_TRUE;
}

PHP_METHOD(imagick, gammaimage)
{
	zval *object;
	php_imagick_object *intern;
	MagickBooleanType status;
	double gamma;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &gamma ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickGammaImage( intern->magick_wand, gamma );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, gammaimagechannel)
{
	zval *object;
	php_imagick_object *intern;
	MagickBooleanType status;
	long channel;
	double gamma;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ld", &channel, &gamma ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickGammaImageChannel( intern->magick_wand, channel, gamma );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, gaussianblurimage)
{
	zval *object;
	php_imagick_object *intern;
	MagickBooleanType status;
	double radius, sigma;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dd", &radius, &sigma ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickGaussianBlurImage( intern->magick_wand, radius, sigma );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, gaussianblurimagechannel)
{
	zval *object;
	php_imagick_object *intern;
	MagickBooleanType status;
	long channel;
	double radius, sigma;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ldd", &channel, &radius, &sigma ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickGaussianBlurImageChannel( intern->magick_wand, channel, radius, sigma );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, compareimages)
{
	MagickWand *tmpWand;
	zval *object, *objvar;
	php_imagick_object *intern, *intern_second, *intern_return;
	long metricType;
	double distortion;
	zval *newWand;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ol", &objvar, php_imagick_sc_entry, &metricType ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_second = (php_imagick_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern_second->magick_wand, 1, 1 );

	MAKE_STD_ZVAL( newWand );
	array_init( return_value );

	object_init_ex( newWand, php_imagick_sc_entry );
	tmpWand = MagickCompareImages( intern->magick_wand, intern_second->magick_wand, metricType, &distortion );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Compare images failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(newWand TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	add_next_index_zval( return_value, newWand );
	add_next_index_double( return_value, distortion );

	return;
}

PHP_METHOD(imagick, contrastimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	zend_bool contrast;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "b", &contrast ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickContrastImage( intern->magick_wand, contrast );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setimagedepth)
{
	php_imagick_object *intern;
	long depth;

	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &depth ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageDepth( intern->magick_wand, depth );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setimagegamma)
{
	php_imagick_object *intern;
	double gamma;

	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &gamma ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageGamma( intern->magick_wand, gamma );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setimageiterations)
{
	php_imagick_object *intern;
	long iterations;

	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &iterations ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageIterations( intern->magick_wand, iterations );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setimagematte)
{
	php_imagick_object *intern;
	zend_bool matte;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "b", &matte ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageMatte( intern->magick_wand, matte );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setimagemattecolor)
{
	zval *object, *objvar;
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &objvar, php_imagickpixel_sc_entry) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	internp = (php_imagickpixel_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	status = MagickSetImageMatteColor( intern->magick_wand, internp->pixel_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, setimagepage)
{
	php_imagick_object *intern;
	long width, height, x, y;

	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "llll", &width, &height, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImagePage( intern->magick_wand, width, height, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setimageresolution)
{
	php_imagick_object *intern;
	double xRes, yRes;

	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &xRes, &yRes ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageResolution( intern->magick_wand, xRes, yRes );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setimagescene)
{
	php_imagick_object *intern;
	long scene;

	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &scene ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageScene( intern->magick_wand, scene );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setimagetickspersecond)
{
	php_imagick_object *intern;
	long ticksPerSecond;

	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &ticksPerSecond ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageTicksPerSecond( intern->magick_wand, ticksPerSecond );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setimagetype)
{
	php_imagick_object *intern;
	long imageType;

	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &imageType ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageType( intern->magick_wand, imageType );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setimageunits)
{
	php_imagick_object *intern;
	long units;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &units ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageType( intern->magick_wand, units );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setimageformat)
{
	char *format;
	int formatLen;
	zval *object;
	MagickBooleanType status;
	php_imagick_object *intern;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &format, &formatLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageFormat( intern->magick_wand, format );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, charcoalimage)
{
	double sigma, radius;
	MagickBooleanType status;
	php_imagick_object *intern;
	zval *object;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &radius, &sigma ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickCharcoalImage( intern->magick_wand, sigma, radius );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, oilpaintimage)
{
	double radius;
	MagickBooleanType status;
	php_imagick_object *intern;
	zval *object;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &radius ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickOilPaintImage( intern->magick_wand, radius );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, normalizeimage)
{
	MagickBooleanType status;
	php_imagick_object *intern;
	zval *object;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickNormalizeImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, normalizeimagechannel)
{
	MagickBooleanType status;
	php_imagick_object *intern;
	zval *object;
	long channel;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &channel ) == FAILURE )
	{
		return;
	}
	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );
	status = MagickNormalizeImageChannel( intern->magick_wand, channel );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, labelimage)
{
	char *label;
	int labelLen;
	zval *object;
	MagickBooleanType status;
	php_imagick_object *intern;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &label, &labelLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );
	status = MagickLabelImage( intern->magick_wand, label );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, writeimage)
{
	char *fileName;
	int fileNameLen;
	zval *object;
	object = getThis();
	MagickBooleanType status;
	php_imagick_object *intern;
	char *absolute;
	char *buffer;

	fileName = (char *)NULL;

	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "|s", &fileName, &fileNameLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	IMAGICK_HAS_FORMAT( buffer, intern->magick_wand );

	if ( fileName == (char *)NULL )
	{
		fileName = MagickGetImageFilename( intern->magick_wand );
	}

	if ( strlen( fileName ) == 0 )
	{
		throwExceptionWithMessage( 1, "No image filename specified.", 1 TSRMLS_CC );
		RETURN_FALSE;
	}

	absolute = expand_filepath( fileName, NULL TSRMLS_CC);

	IMAGICK_SAFE_MODE_CHECK( "Safe mode restricts user to write image: %s", absolute );

	status = MagickWriteImage( intern->magick_wand, absolute );
	efree( absolute );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;

}


PHP_METHOD(imagick, writeimages)
{
	char *fileName;
	zend_bool adjoin;
	int fileNameLen;
	zval *object;
	object = getThis();
	MagickBooleanType status;
	php_imagick_object *intern;
	char *absolute;

	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	if( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "sb", &fileName, &fileNameLen, &adjoin ) == FAILURE )
	{
		return;
	}

	if ( strlen( fileName ) == 0 )
	{
		RETURN_FALSE;
	}

	absolute = expand_filepath( fileName, NULL TSRMLS_CC);
	IMAGICK_SAFE_MODE_CHECK( "Safe mode restricts user to write image: %s", absolute );

	status = MagickWriteImages( intern->magick_wand, fileName, adjoin );
	efree( absolute );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;

}


PHP_METHOD(imagick, drawimage)
{
	zval *object;
	zval *objvar;
	php_imagick_object *intern;
	MagickBooleanType status;
	php_imagickdraw_object *internd;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &objvar, php_imagickdraw_sc_entry) == FAILURE)
	{
		return;
	}

	internd = (php_imagickdraw_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	status = MagickDrawImage( intern->magick_wand, internd->drawing_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, annotateimage)
{
	zval *object;
	php_imagick_object *intern;
	MagickBooleanType status;
	php_imagickdraw_object *internd;
	double x, y, angle;
	char *text;
	int textLen;
	zval *objvar;

	if ( ZEND_NUM_ARGS() != 5 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Oddds", &objvar, php_imagickdraw_sc_entry, &x, &y, &angle, &text, &textLen) == FAILURE)
	{
		return;
	}

	internd = (php_imagickdraw_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	status = MagickAnnotateImage( intern->magick_wand, internd->drawing_wand, x, y, angle, text );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, setimagecompressionquality)
{
	zval *object;
	long quality;
	php_imagick_object *intern;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &quality) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSetImageCompressionQuality( intern->magick_wand, quality );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, compositeimage)
{
	zval *object;
	zval *objvar;
	php_imagick_object *intern;
	php_imagick_object *intern_second;
	long x, y;
	long compositeId = 0;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Olll", &objvar, php_imagick_sc_entry, &compositeId, &x, &y ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_second = (php_imagick_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern_second->magick_wand, 1, 1 );

	MagickCompositeImage( intern->magick_wand, intern_second->magick_wand, compositeId, x, y );
	RETURN_TRUE;
}

PHP_METHOD(imagick, compositeimagechannel)
{
	zval *object;
	zval *objvar;
	php_imagick_object *intern;
	php_imagick_object *intern_second;
	long x, y, channel, compositeId;

	if ( ZEND_NUM_ARGS() != 5 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ollll", &objvar, php_imagick_sc_entry, &channel, &compositeId, &x, &y ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_second = (php_imagick_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern_second->magick_wand, 1, 1 );

	MagickCompositeImageChannel( intern->magick_wand, channel, intern_second->magick_wand, compositeId, x, y );
	RETURN_TRUE;
}

PHP_METHOD(imagick, modulateimage)
{
	zval *object;
	php_imagick_object *intern;
	MagickBooleanType status;
	double brightness, saturation, hue;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ddd", &brightness, &saturation, &hue ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickModulateImage( intern->magick_wand, brightness, saturation, hue );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, addnoiseimage)
{
	zval *object;
	php_imagick_object *intern;
	MagickBooleanType status;
	long noise;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &noise ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickAddNoiseImage( intern->magick_wand, noise );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, addnoiseimagechannel)
{
	zval *object;
	php_imagick_object *intern;
	MagickBooleanType status;
	long channel, noise;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &channel, &noise ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickAddNoiseImageChannel( intern->magick_wand, channel, noise );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, adaptiveresizeimage)
{
	zval *object;
	php_imagick_object *intern;
	MagickBooleanType status;
	long columns, rows;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &columns, &rows ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickAdaptiveResizeImage( intern->magick_wand, columns, rows );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, adaptivesharpenimage)
{
	zval *object;
	php_imagick_object *intern;
	MagickBooleanType status;
	double radius, sigma;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dd", &radius, &sigma ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickAdaptiveSharpenImage( intern->magick_wand, radius, sigma );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, adaptivesharpenimagechannel)
{
	zval *object;
	php_imagick_object *intern;
	MagickBooleanType status;
	long channel;
	double radius, sigma;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ldd", &channel, &radius, &sigma ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickAdaptiveSharpenImageChannel( intern->magick_wand, channel, radius, sigma );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, montageimage)
{
	MagickWand *tmpWand;
	zval *object, *objvar;
	php_imagick_object *intern, *intern_second;
	php_imagickdraw_object *internd;
	char *tileGeom, *thumbnailGeom, *frame;
	int tileGeomLen, thumbnailGeomLen, frameLen;
	long montageMode = 0;

	if ( ZEND_NUM_ARGS() != 5 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ossls", &objvar, php_imagickdraw_sc_entry,
		&tileGeom, &tileGeomLen,
		&thumbnailGeom, &thumbnailGeomLen,
		&montageMode,
		&frame, &frameLen ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	internd = (php_imagickdraw_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	object_init_ex( return_value, php_imagick_sc_entry );

	tmpWand = MagickMontageImage( intern->magick_wand, internd->drawing_wand, tileGeom, thumbnailGeom, montageMode, frame );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Montage image failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_second = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_second->magick_wand = tmpWand;

	return;
}

PHP_METHOD(imagick, affinetransformimage)
{
	zval *object, *objvar;
	php_imagick_object *intern;
	php_imagickdraw_object *internd;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &objvar, php_imagickdraw_sc_entry ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	internd = (php_imagickdraw_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	status = MagickAffineTransformImage( intern->magick_wand, internd->drawing_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, averageimages)
{
	MagickWand *tmpWand;
	zval *object;
	php_imagick_object *intern, *intern_second;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickAverageImages( intern->magick_wand );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "AffineTransform image failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_second = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_second->magick_wand = tmpWand;
	return;
}

PHP_METHOD(imagick, borderimage)
{
	zval *object, *objvar;
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	MagickBooleanType status;
	long width, height;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Oll", &objvar, php_imagickpixel_sc_entry, &width, &height ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	internp = (php_imagickpixel_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	status = MagickBorderImage( intern->magick_wand, internp->pixel_wand, width, height );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, thresholdimage)
{
	php_imagick_object *intern;
	double threshold;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &threshold) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickThresholdImage( intern->magick_wand, threshold );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, thresholdimagechannel)
{
	php_imagick_object *intern;
	long channel;
	double threshold;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ld", &channel, &threshold) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickThresholdImageChannel( intern->magick_wand, channel, threshold );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, adaptivethresholdimage)
{
	php_imagick_object *intern;
	long width, height, offset;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &width, &height, &offset) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickAdaptiveThresholdImage( intern->magick_wand, width, height, offset );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, sharpenimage)
{
	double sigma, radius;
	MagickBooleanType status;
	php_imagick_object *intern;
	zval *object;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &radius, &sigma ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSharpenImage( intern->magick_wand, sigma, radius );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, sharpenimagechannel)
{
	long channel;
	double sigma, radius;
	MagickBooleanType status;
	php_imagick_object *intern;
	zval *object;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ldd", &channel, &radius, &sigma ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSharpenImageChannel( intern->magick_wand, channel, sigma, radius );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;

}

PHP_METHOD(imagick, shaveimage)
{
	php_imagick_object *intern;
	long columns, rows;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &columns, &rows) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickShaveImage( intern->magick_wand, columns, rows );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, shearimage)
{
	zval *object;
	zval *objvar;
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	double xShear, yShear;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Odd", &objvar, php_imagickpixel_sc_entry, &xShear, &yShear) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	internp = (php_imagickpixel_object *)zend_object_store_get_object(objvar TSRMLS_CC);

	status = MagickShearImage( intern->magick_wand, internp->pixel_wand, xShear, yShear );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, sketchimage)
{
	double radius, sigma, angle;
	MagickBooleanType status;
	php_imagick_object *intern;
	zval *object;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ddd", &radius, &sigma, &angle ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSketchImage( intern->magick_wand, sigma, radius, angle );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, spliceimage)
{
	long width, height, x, y;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "llll", &width, &height, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSpliceImage( intern->magick_wand, width, height, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, steganoimage)
{
	zval *object, *objvar;
	php_imagick_object *intern, *intern_second, *intern_return;
	long offset;
	MagickWand *tmpWand;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ol", &objvar, php_imagick_sc_entry, &offset) == FAILURE)
	{
		return;
	}

	object = getThis();

	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	intern_second = (php_imagick_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern_second->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickSteganoImage( intern->magick_wand, intern_second->magick_wand, offset );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Stegano image failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;
	return;
}

PHP_METHOD(imagick, clone)
{
	zval *object;
	php_imagick_object *intern, *intern_return;
	MagickWand *tmpWand;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = CloneMagickWand( intern->magick_wand );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Cloning MagickWand failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;
	return;
}

PHP_METHOD(imagick, randomthresholdimage)
{
	php_imagick_object *intern;
	double low, high;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dd", &low, &high) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickRandomThresholdImage( intern->magick_wand, low, high );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, randomthresholdimagechannel)
{
	php_imagick_object *intern;
	long channel;
	double low, high;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ldd", &channel, &low, &high) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickRandomThresholdImageChannel( intern->magick_wand, channel, low, high );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, rotateimage)
{
	zval *object;
	zval *objvar;
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	double degrees;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Od", &objvar, php_imagickpixel_sc_entry, &degrees) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	internp = (php_imagickpixel_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	status = MagickRotateImage( intern->magick_wand, internp->pixel_wand, degrees );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, sampleimage)
{
	php_imagick_object *intern;
	long columns, rows;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &columns, &rows) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSampleImage( intern->magick_wand, columns, rows );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, solarizeimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	long threshold;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &threshold ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickSolarizeImage( intern->magick_wand, threshold );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, shadeimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	double azimuth, elevation;
	zend_bool gray;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "bdd", &gray, &azimuth, &elevation ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickShadeImage( intern->magick_wand, gray, azimuth, elevation );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, shadowimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	double opacity, sigma;
	long x, y;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ddll", &opacity, &sigma, &x, &y ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickShadowImage( intern->magick_wand, opacity, sigma, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, motionblurimage)
{

	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	double radius, sigma, angle;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ddd", &radius, &sigma, &angle ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickMotionBlurImage( intern->magick_wand, radius, sigma, angle );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;


}

PHP_METHOD(imagick, mosaicimages)
{
	MagickWand *tmpWand;
	zval *object;
	php_imagick_object *intern, *intern_return;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickMosaicImages( intern->magick_wand );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Mosaic image failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	return;

}

PHP_METHOD(imagick, morphimages)
{
	MagickWand *tmpWand;
	zval *object;
	php_imagick_object *intern, *intern_return;
	long frames;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &frames ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	tmpWand = MagickMorphImages( intern->magick_wand, frames );

	if ( !IsMagickWand( tmpWand ) )
	{
		throwExceptionWithMessage( 1, "Mosaic image failed.", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_return = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	intern_return->magick_wand = tmpWand;

	return;

}

PHP_METHOD(imagick, minifyimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickMinifyImage( intern->magick_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;

}



PHP_METHOD(imagick, posterizeimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	long levels;
	zend_bool dither;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &levels, &dither ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickPosterizeImage( intern->magick_wand, levels, dither );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;

}

PHP_METHOD(imagick, radialblurimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	double angle;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &angle) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickRadialBlurImage( intern->magick_wand, angle );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, radialblurimagechannel)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	long channel;
	double angle;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ld", &channel, &angle) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickRadialBlurImageChannel( intern->magick_wand, channel, angle );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, raiseimage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	long width, height, x, y;
	zend_bool raise;

	if ( ZEND_NUM_ARGS() != 5 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llllb", &width, &height, &x, &y, &raise ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = MagickRaiseImage( intern->magick_wand, width, height, x, y, raise );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, blackthresholdimage)
{
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	zval *object, *objvar;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &objvar, php_imagickpixel_sc_entry) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	internp = (php_imagickpixel_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	status = MagickBlackThresholdImage( intern->magick_wand, internp->pixel_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, resampleimage)
{
	double xRes, yRes, blur;
	long filter = 0;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ddld", &xRes, &yRes, &filter, &blur ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );
	status = MagickResampleImage( intern->magick_wand, xRes, yRes, filter, blur );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, resizeimage)
{
	double columns, rows, blur;
	long filter = 0;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() !=4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ddld", &columns, &rows, &filter, &blur ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );
	status = MagickResizeImage( intern->magick_wand, columns, rows, filter, blur );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, rollimage)
{
	long x, y;
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &x, &y ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );
	status = MagickRollImage( intern->magick_wand, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, appendimages)
{
	php_imagick_object *intern, *intern_second;
	zval *object;
	MagickBooleanType status;
	MagickWand *newWand;
	zend_bool stack;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &stack) == FAILURE)
	{
		return;
	}

	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object_init_ex( return_value, php_imagick_sc_entry );
	object = getThis();

	intern_second = (php_imagick_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	newWand = MagickAppendImages( intern->magick_wand, stack );

	if ( !IsMagickWand( newWand ) )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern_second->magick_wand = newWand;

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	return;
}


PHP_METHOD(imagick, whitethresholdimage)
{
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	zval *object, *objvar;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &objvar, php_imagickpixel_sc_entry) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	internp = (php_imagickpixel_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	status = MagickWhiteThresholdImage( intern->magick_wand, internp->pixel_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagick, getpixeliterator)
{
	php_imagick_object *intern;
	zval *object;
	zval retval, *methodArray;
	zval *args[1];
	zval *tmpObject;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	MAKE_STD_ZVAL( tmpObject );

	object_init_ex( tmpObject, php_imagickpixeliterator_sc_entry );

	MAKE_STD_ZVAL( methodArray );
	array_init( methodArray );

	add_next_index_zval( methodArray, tmpObject );
	add_next_index_string( methodArray, "newpixeliterator", 1 );

	args[0] = object;
	call_user_function( EG(function_table), NULL, methodArray, &retval, 1, args TSRMLS_CC);

	*return_value = *tmpObject;
	zval_copy_ctor(return_value);

	zval_dtor( methodArray );
	FREE_ZVAL( methodArray );

	return;

}


PHP_METHOD(imagick, getpixelregioniterator)
{
	php_imagick_object *intern;
	zval *object;
	zval retval, *methodArray;
	zval *args[5];
	zval *tmpObject;
	zval *x, *y, *columns, *rows;

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "zzzz", &x, &y, &columns, &rows ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	MAKE_STD_ZVAL( tmpObject );

	object_init_ex( tmpObject, php_imagickpixeliterator_sc_entry );

	MAKE_STD_ZVAL( methodArray );
	array_init( methodArray );

	add_next_index_zval( methodArray, tmpObject );
	add_next_index_string( methodArray, "newpixelregioniterator", 1 );

	args[0] = object;
	args[1] = x;
	args[2] = y;
	args[3] = columns;
	args[4] = rows;
	call_user_function( EG(function_table), NULL, methodArray, &retval, 5, args TSRMLS_CC);

	*return_value = *tmpObject;
	zval_copy_ctor(return_value);

	zval_dtor( methodArray );
	FREE_ZVAL( methodArray );

	return;

}

PHP_METHOD(imagick, getcompression)
{
	php_imagick_object *intern;
	zval *object;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	RETVAL_LONG( MagickGetCompression( intern->magick_wand ) );
}

PHP_METHOD(imagick, getcompressionquality)
{
	php_imagick_object *intern;
	zval *object;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	RETVAL_LONG( MagickGetCompressionQuality( intern->magick_wand ) );
}

PHP_METHOD(imagick, getcopyright)
{
	php_imagick_object *intern;
	zval *object;
	char *copyright;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	copyright = (char *)MagickGetCopyright();
	ZVAL_STRING( return_value, copyright, 1 );

	IMAGICK_FREE_MEMORY( char *, copyright );
	return;
}

PHP_METHOD(imagick, getfilename)
{
	php_imagick_object *intern;
	zval *object;
	char *filename;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	filename = (char *)MagickGetFilename( intern->magick_wand );
	ZVAL_STRING( return_value, filename, 1 );

	IMAGICK_FREE_MEMORY( char *, filename );
	return;
}

PHP_METHOD(imagick, getformat)
{
	php_imagick_object *intern;
	zval *object;
	char *format;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	format = (char *)MagickGetFormat( intern->magick_wand );
	ZVAL_STRING( return_value, format, 1 );

	IMAGICK_FREE_MEMORY( char *, format );
	return;
}

PHP_METHOD(imagick, gethomeurl)
{
	php_imagick_object *intern;
	zval *object;
	char *homeURL;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	homeURL = (char *)MagickGetHomeURL();
	ZVAL_STRING( return_value, homeURL, 1 );

	IMAGICK_FREE_MEMORY( char *, homeURL );
	return;
}

PHP_METHOD(imagick, getinterlacescheme)
{
	php_imagick_object *intern;
	zval *object;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	RETVAL_LONG( MagickGetInterlaceScheme( intern->magick_wand ) );
}

PHP_METHOD(imagick, getoption)
{
	php_imagick_object *intern;
	zval *object;
	char *key, *value;
	int keyLen;


	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &keyLen ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	value = MagickGetOption( intern->magick_wand, key );
	ZVAL_STRING( return_value, value, 1 );

	IMAGICK_FREE_MEMORY( char *, value );
	return;
}

PHP_METHOD(imagick, getpackagename)
{
	php_imagick_object *intern;
	zval *object;
	char *packageName;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	packageName = (char *)MagickGetPackageName();
	ZVAL_STRING( return_value, packageName, 1 );

	IMAGICK_FREE_MEMORY( char *, packageName );
	return;
}

PHP_METHOD(imagick, getpage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	long width, height, x, y;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	status = MagickGetPage( intern->magick_wand, &width, &height, &x, &y );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC );
		RETURN_FALSE;
	}

	array_init( return_value );
	add_assoc_long( return_value, "width", width );
	add_assoc_long( return_value, "height", height );
	add_assoc_long( return_value, "x", x );
	add_assoc_long( return_value, "y", y );

	return;
}

PHP_METHOD(imagick, getquantumdepth)
{
	php_imagick_object *intern;
	zval *object;
	char *quantumDepth;
	long depth;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	quantumDepth = (char *)MagickGetQuantumDepth( &depth );
	array_init( return_value );

	add_assoc_long( return_value, "quantumDepthLong", depth );
	add_assoc_string( return_value, "quantumDepthString", quantumDepth, 1 );

	IMAGICK_FREE_MEMORY( char *, quantumDepth );
	return;
}

PHP_METHOD(imagick, getquantumrange)
{
	php_imagick_object *intern;
	zval *object;
	char *quantumRange;
	long range;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	quantumRange = (char *)MagickGetQuantumRange( &range );
	array_init( return_value );

	add_assoc_long( return_value, "quantumRangeLong", range );
	add_assoc_string( return_value, "quantumRangeString", quantumRange, 1 );

	IMAGICK_FREE_MEMORY( char *, quantumRange );
	return;
}

PHP_METHOD(imagick, getreleasedate)
{
	php_imagick_object *intern;
	zval *object;
	char *releaseDate;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	releaseDate = (char *)MagickGetPackageName();
	ZVAL_STRING( return_value, releaseDate, 1 );

	IMAGICK_FREE_MEMORY( char *, releaseDate );
	return;
}

PHP_METHOD(imagick, getresource)
{
	php_imagick_object *intern;
	zval *object;
	long resourceType;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &resourceType ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	RETVAL_LONG( MagickGetResource( resourceType ) );
}

PHP_METHOD(imagick, getresourcelimit)
{
	php_imagick_object *intern;
	zval *object;
	long resourceType;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &resourceType ) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	RETVAL_LONG( MagickGetResourceLimit( resourceType ) );
}

PHP_METHOD(imagick, getsamplingfactors)
{
	php_imagick_object *intern;
	zval *object;
	double *samplingFactors;
	long numberFactors, i;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	samplingFactors = (double *)MagickGetSamplingFactors( intern->magick_wand, &numberFactors );

	array_init( return_value );

	for ( i = 0 ; i < numberFactors; i++ )
	{
		add_next_index_double( return_value, samplingFactors[i] );
	}

	return;
}

PHP_METHOD(imagick, getsize)
{
	php_imagick_object *intern;
	zval *object;
	unsigned long columns, rows;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	status = MagickGetSize( intern->magick_wand, &columns, &rows );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC );
		RETURN_FALSE;
	}

	array_init( return_value );
	add_assoc_long( return_value, "columns", columns );
	add_assoc_long( return_value, "rows", rows );

	return;
}

PHP_METHOD(imagick, getsizeoffset)
{
	php_imagick_object *intern;
	zval *object;
	long offset;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );
	status = MagickGetSizeOffset( intern->magick_wand, &offset );

	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC );
		RETURN_FALSE;
	}

	RETVAL_LONG( offset );
}


PHP_METHOD(imagick, getversion)
{
	php_imagick_object *intern;
	zval *object;
	char *versionString;
	long versionNumber;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagick_object *, intern );

	versionString = (char *)MagickGetVersion( &versionNumber );
	array_init( return_value );

	add_assoc_long( return_value, "versionNumber", versionNumber );
	add_assoc_string( return_value, "versionString", versionString, 1 );

	IMAGICK_FREE_MEMORY( char *, versionString );
	return;
}

PHP_METHOD(imagick, setbackgroundcolor)
{
	zval *object;
	zval *objvar;
	php_imagick_object *intern;
	php_imagickpixel_object *internp;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &objvar, php_imagickpixel_sc_entry) == FAILURE)
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);
	internp = (php_imagickpixel_object *)zend_object_store_get_object(objvar TSRMLS_CC);

	status = MagickSetBackgroundColor( intern->magick_wand, internp->pixel_wand );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, setcompression)
{
	php_imagick_object *intern;
	long compression;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &compression ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickSetCompression( intern->magick_wand, compression );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setcompressionquality)
{
	php_imagick_object *intern;
	long quality;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &quality ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickSetCompressionQuality( intern->magick_wand, quality );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setfilename)
{
	php_imagick_object *intern;
	char *fileName;
	int fileNameLen;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &fileName, &fileNameLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickSetFilename( intern->magick_wand, fileName );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, setformat)
{
	php_imagick_object *intern;
	char *format;
	int formatLen;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &format, &formatLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickSetFormat( intern->magick_wand, format );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagick, setinterlacescheme)
{
	php_imagick_object *intern;
	long schema;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &schema ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickSetInterlaceScheme( intern->magick_wand, schema );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setoption)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	char *key, *value;
	int keyLen, valueLen;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ss", &key, &keyLen, &value, &valueLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickSetOption( intern->magick_wand, key, value );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setpage)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	long width, height, x, y;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "llll", &width, &height, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickSetPage( intern->magick_wand, width, height, x, y );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setresourcelimit)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	long type, limit;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ll", &type, &limit ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickSetResourceLimit( type, limit );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, setresolution)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	double xResolution, yResolution;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &xResolution, &yResolution ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickSetResolution( intern->magick_wand, xResolution, yResolution );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setsamplingfactors)
{
	php_imagick_object *intern;
	zval *object;
	MagickBooleanType status;
	zval *factors;
	double *dArray;
	long elements = 0;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "a", &factors ) == FAILURE )
	{
		return;
	}

	dArray = getDoublesFromZval( factors, &elements TSRMLS_CC );

	if ( dArray == (double *)NULL )
	{
		throwExceptionWithMessage( 2, "Can't read array.", 2 TSRMLS_CC );
		RETURN_FALSE;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickSetSamplingFactors( intern->magick_wand, elements, dArray );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setsize)
{
	php_imagick_object *intern;
	long columns, rows;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ll", &columns, &rows ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickSetSize( intern->magick_wand, columns, rows );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}


PHP_METHOD(imagick, setsizeoffset)
{
	php_imagick_object *intern;
	long columns, rows, offset;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "lll", &columns, &rows, &offset ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickSetSizeOffset( intern->magick_wand, columns, rows, offset );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(imagick, settype)
{
	php_imagick_object *intern;
	long type;
	zval *object;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &type ) == FAILURE )
	{
		return;
	}

	object = getThis();
	intern = (php_imagick_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = MagickSetType( intern->magick_wand, type );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickException( intern->magick_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

/* end of magickwand*/

PHP_METHOD(imagickdraw, __construct)
{

}


PHP_METHOD(imagickdraw, circle)
{
	zval *object;
	double ox, oy, px, py;
	php_imagickdraw_object *internd;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddd", &ox, &oy, &px, &py ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawCircle( internd->drawing_wand, ox, oy, px, py );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, rectangle)
{
	zval *object;
	double x1, y1, x2, y2;
	php_imagickdraw_object *internd;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddd", &x1, &y1, &x2, &y2 ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawRectangle( internd->drawing_wand, x1, y1, x2, y2 );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, roundrectangle)
{
	zval *object;
	double x1, y1, x2, y2, rx, ry;
	php_imagickdraw_object *internd;

	if ( ZEND_NUM_ARGS() != 6 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddddd", &x1, &y1, &x2, &y2, &rx, &ry ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawRoundRectangle( internd->drawing_wand, x1, y1, x2, y2, rx, ry );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, ellipse)
{
	zval *object;
	double ox, oy, rx, ry, start, end;
	php_imagickdraw_object *internd;

	if ( ZEND_NUM_ARGS() != 6 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddddd", &ox, &oy, &rx, &ry, &start, &end ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawEllipse( internd->drawing_wand, ox, oy, rx, ry, start, end );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, skewx)
{

	zval *object;
	double degrees;
	php_imagickdraw_object *internd;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &degrees ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawSkewX( internd->drawing_wand, degrees );
	RETURN_TRUE;

}


PHP_METHOD(imagickdraw, skewy)
{

	zval *object;
	double degrees;
	php_imagickdraw_object *internd;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &degrees ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawSkewY( internd->drawing_wand, degrees );
	RETURN_TRUE;

}


PHP_METHOD(imagickdraw, translate)
{

	zval *object;
	double x, y;
	php_imagickdraw_object *internd;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawTranslate( internd->drawing_wand, x, y );
	RETURN_TRUE;

}


PHP_METHOD(imagickdraw, setfillcolor)
{
	zval *object;
	zval *objvar;
	php_imagickdraw_object *internd;
	php_imagickpixel_object *internp;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &objvar, php_imagickpixel_sc_entry) == FAILURE)
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	internp = (php_imagickpixel_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	DrawSetFillColor( internd->drawing_wand, internp->pixel_wand );

	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, setstrokecolor)
{
	zval *object;
	zval *objvar;
	php_imagickdraw_object *internd;
	php_imagickpixel_object *internp;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &objvar, php_imagickpixel_sc_entry) == FAILURE)
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	internp = (php_imagickpixel_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	DrawSetStrokeColor( internd->drawing_wand, internp->pixel_wand );

	RETURN_TRUE;

}


PHP_METHOD(imagickdraw, setfillalpha)
{
	zval *object;
	php_imagickdraw_object *internd;
	double opacity;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &opacity) == FAILURE)
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawSetFillAlpha( internd->drawing_wand, opacity );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, settextantialias)
{
	zval *object;
	php_imagickdraw_object *internd;
	zend_bool antiAlias;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &antiAlias) == FAILURE)
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawSetTextAntialias( internd->drawing_wand, antiAlias );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, settextencoding)
{
	zval *object;
	php_imagickdraw_object *internd;
	char *encoding;
	int encodingLen;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &encoding, &encodingLen) == FAILURE)
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawSetTextEncoding( internd->drawing_wand, encoding );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, setstrokealpha)
{
	zval *object;
	php_imagickdraw_object *internd;
	double opacity;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &opacity) == FAILURE)
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawSetStrokeAlpha( internd->drawing_wand, opacity );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, setstrokewidth)
{
	zval *object;
	php_imagickdraw_object *internd;
	double width;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &width) == FAILURE)
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawSetStrokeWidth( internd->drawing_wand, width );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, setfont)
{
	zval *object;
	php_imagickdraw_object *internd;
	char *font;
	int fontLen;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &font, &fontLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = DrawSetFont( internd->drawing_wand, font );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickDrawException( internd->drawing_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, setfontfamily)
{
	zval *object;
	php_imagickdraw_object *internd;
	char *fontFamily;
	int fontFamilyLen;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &fontFamily, &fontFamilyLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = DrawSetFontFamily( internd->drawing_wand, fontFamily );

	/* No magick is going to happen */
	if ( status == MagickFalse )
	{
		throwImagickDrawException( internd->drawing_wand, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, setfontsize)
{
	zval *object;
	php_imagickdraw_object *internd;
	double fontSize;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &fontSize ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawSetFontSize( internd->drawing_wand, fontSize );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, setfontstyle)
{
	zval *object;
	php_imagickdraw_object *internd;
	long styleId = 0;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &styleId ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawSetFontStyle( internd->drawing_wand, styleId );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, setfontweight)
{
	zval *object;
	php_imagickdraw_object *internd;
	long weight;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &weight ) == FAILURE )
	{
		return;
	}

	if ( weight >= 100 && weight <= 900)
	{
		object = getThis();
		internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

		DrawSetFontWeight( internd->drawing_wand, weight );
		RETURN_TRUE;
	}
	else
	{
		throwExceptionWithMessage( 4, "Font weight valid range is 100-900.", 4 TSRMLS_CC);
		RETURN_FALSE;
	}
}


PHP_METHOD(imagickdraw, setfontstretch)
{
	zval *object;
	php_imagickdraw_object *internd;
	long stretch;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &stretch ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawSetFontStretch( internd->drawing_wand, stretch );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, setstrokeantialias)
{
	zval *object;
	php_imagickdraw_object *internd;
	zend_bool antiAlias;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "b", &antiAlias ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawSetTextAntialias( internd->drawing_wand, antiAlias );
	RETURN_TRUE;

}

PHP_METHOD(imagickdraw, settextalignment)
{
	zval *object;
	php_imagickdraw_object *internd;
	long align;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &align ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawSetTextAlignment( internd->drawing_wand, align );
	RETURN_TRUE;

}

PHP_METHOD(imagickdraw, settextdecoration)
{
	zval *object;
	php_imagickdraw_object *internd;
	long decoration;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &decoration ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawSetTextDecoration( internd->drawing_wand, decoration );
	RETURN_TRUE;

}

PHP_METHOD(imagickdraw, settextundercolor)
{
	zval *object;
	zval *objvar;
	php_imagickdraw_object *internd;
	php_imagickpixel_object *internp;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &objvar, php_imagickpixel_sc_entry) == FAILURE)
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	internp = (php_imagickpixel_object *)zend_object_store_get_object(objvar TSRMLS_CC);
	DrawSetTextUnderColor( internd->drawing_wand, internp->pixel_wand );

	RETURN_TRUE;

}

PHP_METHOD(imagickdraw, setviewbox)
{
	zval *object;
	php_imagickdraw_object *internd;
	long x1, y1, x2, y2;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "llll", &x1, &y1, &x2, &y2 ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawSetViewbox( internd->drawing_wand, x1, y1, x2, y2 );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, getfont)
{
	zval *object;
	php_imagickdraw_object *internd;
	char *font;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );

	font = DrawGetFont( internd->drawing_wand );
	ZVAL_STRING( return_value, font, 1 );

	IMAGICK_FREE_MEMORY( char *, font );
	return;
}


PHP_METHOD(imagickdraw, getfontfamily)
{
	zval *object;
	php_imagickdraw_object *internd;
	char *fontFamily;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );

	fontFamily = DrawGetFontFamily( internd->drawing_wand );
	ZVAL_STRING( return_value, fontFamily, 1 );

	IMAGICK_FREE_MEMORY( char *, fontFamily );
	return;
}


PHP_METHOD(imagickdraw, getfontsize)
{
	zval *object;
	php_imagickdraw_object *internd;
	double fontSize;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );

	fontSize = DrawGetFontSize( internd->drawing_wand );
	ZVAL_DOUBLE( return_value, fontSize );
	return;
}


PHP_METHOD(imagickdraw, getfontstyle)
{
	zval *object;
	php_imagickdraw_object *internd;
	long fontStyle;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );

	fontStyle = DrawGetFontStyle( internd->drawing_wand );
	ZVAL_LONG( return_value, fontStyle );
	return;
}


PHP_METHOD(imagickdraw, getfontweight)
{
	zval *object;
	php_imagickdraw_object *internd;
	long weight;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );

	weight = DrawGetFontWeight( internd->drawing_wand );
	ZVAL_LONG( return_value, weight );
	return;
}




PHP_METHOD(imagickdraw, clear)
{
	zval *object;
	php_imagickdraw_object *internd;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );

	status = IsDrawingWand( internd->drawing_wand );

	if ( status == MagickFalse )
	{
		throwExceptionWithMessage( 2, "DrawingWand is not allocated.", 2 TSRMLS_CC);
		RETURN_FALSE;
	}

	ClearDrawingWand( internd->drawing_wand );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, gettextdecoration)
{
	zval *object;
	php_imagickdraw_object *internd;
	long decoration;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );

	decoration = DrawGetTextDecoration( internd->drawing_wand );
	ZVAL_LONG( return_value, decoration );
	return;

}

PHP_METHOD(imagickdraw, gettextencoding)
{
	zval *object;
	php_imagickdraw_object *internd;
	char *encoding;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );

	encoding = DrawGetTextEncoding( internd->drawing_wand );
	ZVAL_STRING( return_value, encoding, 1 );
	IMAGICK_FREE_MEMORY( char *, encoding );

	return;
}



PHP_METHOD(imagickdraw, destroy)
{

	zval *object;
	php_imagickdraw_object *internd;
	MagickBooleanType status;

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = IsDrawingWand( internd->drawing_wand );

	if ( status == MagickFalse )
	{
		throwExceptionWithMessage( 2, "DrawingWand is not allocated.", 2 TSRMLS_CC);
		RETURN_FALSE;
	}
	ClearDrawingWand( internd->drawing_wand );
	internd->drawing_wand = DestroyDrawingWand( internd->drawing_wand );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, annotation)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x, y;
	unsigned char *text;
	int textLen;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dds", &x, &y, &text, &textLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawAnnotation( internd->drawing_wand, x, y, text );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, arc)
{
	double sx, sy, ex, ey, sd, ed;
	zval *object;
	php_imagickdraw_object *internd;

	if ( ZEND_NUM_ARGS() != 6 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddddd", &sx, &sy, &ex, &ey, &sd, &ed ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawArc( internd->drawing_wand, sx, sy, ex, ey, sd, ed );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, matte)
{
	double x, y;
	zval *object;
	php_imagickdraw_object *internd;
	long paintMethod;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ddl", &x, &y, &paintMethod ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawMatte( internd->drawing_wand, x, y, paintMethod );
	RETURN_TRUE;
}


void *getPointInfoFromZval( zval *coordinateArray, int *numElements TSRMLS_DC)
{
    PointInfo *coordinates;
	long elements, subElements, i;
	HashTable *coords;
	zval **ppzval, **ppzX, **ppzY;
	HashTable *subArray;

	elements = zend_hash_num_elements( Z_ARRVAL_P( coordinateArray ) );

	if ( elements < 1 )
	{
		coordinates = (PointInfo *)NULL;
		return coordinates;
	}

	*numElements = elements;
	coordinates = emalloc( sizeof( PointInfo ) * elements );

	coords = Z_ARRVAL_P( coordinateArray );
	zend_hash_internal_pointer_reset_ex( coords, (HashPosition *) 0 );

	for ( i = 0 ; i < elements ; i++ )
	{
		/* Get the sub array */
		if ( zend_hash_get_current_data( coords, (void**)&ppzval ) == FAILURE )
		{
			coordinates = (PointInfo *)NULL;
			return coordinates;
		}

		/* If its something than array lets error here */
		if( Z_TYPE_PP( ppzval ) != IS_ARRAY )
		{
			coordinates = (PointInfo *)NULL;
			return coordinates;
		}

		/* Subarray should have two elements. X and Y */
		subElements = zend_hash_num_elements( Z_ARRVAL_PP( ppzval ) );

		/* Exactly two elements */
		if ( subElements != 2 )
		{
			coordinates = (PointInfo *)NULL;
			return coordinates;
		}

		/* Subarray values */
		subArray = Z_ARRVAL_PP( ppzval );

		/* Get X */
		if ( zend_hash_find( subArray, "x", sizeof( "x" ), (void**)&ppzX ) == FAILURE )
		{
			coordinates = (PointInfo *)NULL;
			return coordinates;
		}

		if( Z_TYPE_PP( ppzX ) != IS_DOUBLE && Z_TYPE_PP( ppzX ) != IS_LONG )
		{
			coordinates = (PointInfo *)NULL;
			return coordinates;
		}

		/* Get Y */
		if ( zend_hash_find( subArray, "y", sizeof( "y" ), (void**)&ppzY ) == FAILURE )
		{
			coordinates = (PointInfo *)NULL;
			return coordinates;
		}

		if( Z_TYPE_PP( ppzY ) != IS_DOUBLE && Z_TYPE_PP( ppzY ) != IS_LONG )
		{
			coordinates = (PointInfo *)NULL;
			return coordinates;
		}

		/* Assign X and Y */

		if ( Z_TYPE_PP( ppzX ) == IS_LONG )
		{
			coordinates[i].x = (double)Z_LVAL_PP( ppzX );
		}
		else
		{
			coordinates[i].x = Z_DVAL_PP( ppzX );
		}

		if ( Z_TYPE_PP( ppzY ) == IS_LONG )
		{
			coordinates[i].y = (double)Z_LVAL_PP( ppzY );
		}
		else
		{
			coordinates[i].y = Z_DVAL_PP( ppzY );
		}
		zend_hash_move_forward( coords );
	}

	return coordinates;
}

PHP_METHOD(imagickdraw, polygon)
{
	zval *coordinateArray, *object;
	php_imagickdraw_object *internd;
	PointInfo *coordinates;
	int numElements = 0;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "a", &coordinateArray ) == FAILURE )
	{
		return;
	}

	coordinates = getPointInfoFromZval( coordinateArray, &numElements TSRMLS_CC );

	if ( coordinates == (PointInfo *)NULL )
	{
		efree( coordinates );
		throwExceptionWithMessage( 1, "Unable to read coordinate array.", 1 TSRMLS_CC );
		RETURN_FALSE;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);
	DrawPolygon( internd->drawing_wand, numElements, coordinates );

	efree( coordinates );

	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, bezier)
{
	zval *coordinateArray, *object;
	php_imagickdraw_object *internd;
	PointInfo *coordinates;
	int numElements = 0;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "a", &coordinateArray ) == FAILURE )
	{
		return;
	}

	coordinates = getPointInfoFromZval( coordinateArray, &numElements TSRMLS_CC );

	if ( coordinates == (PointInfo *)NULL )
	{
		efree( coordinates );
		throwExceptionWithMessage( 1, "Unable to read coordinate array.", 1 TSRMLS_CC );
		RETURN_FALSE;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);
	DrawBezier( internd->drawing_wand, numElements, coordinates );

	efree( coordinates );

	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, point)
{

	zval *object;
	php_imagickdraw_object *internd;
	double x, y;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &x, &y) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);
	DrawPoint( internd->drawing_wand, x, y );

	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, line)
{
	zval *object;
	php_imagickdraw_object *internd;
	double sx, sy, ex, ey;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddd", &sx, &sy, &ex, &ey ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);

	DrawLine( internd->drawing_wand, sx, sy, ex, ey );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, clone)
{
	zval *object;
	php_imagickdraw_object *internd, *intern_return;
  DrawingWand *tmpWand;

  IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );

  object_init_ex( return_value, php_imagickdraw_sc_entry );
  tmpWand = CloneDrawingWand( internd->drawing_wand );

  intern_return = (php_imagickdraw_object *)zend_object_store_get_object(return_value TSRMLS_CC);
  intern_return->drawing_wand = tmpWand;

  return;
}

PHP_METHOD(imagickdraw, affine)
{
	zval *object;
	php_imagickdraw_object *internd;
	zval *affineMatrix, **ppzval;
	HashTable *affine;
	char *matrixElements[] = { "sx", "rx", "ry",
						       "sy", "tx", "ty" };
	int i;
	double value;

	AffineMatrix *pmatrix;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "a", &affineMatrix ) == FAILURE )
	{
		return;
	}

	/* Allocate space to build matrix */
	pmatrix = emalloc( sizeof( AffineMatrix ) );

	affine = Z_ARRVAL_P( affineMatrix );
	zend_hash_internal_pointer_reset_ex( affine, (HashPosition *) 0 );

	for ( i = 0; i < 6 ; i++ )
	{
		if ( zend_hash_find( affine, matrixElements[i], 3, (void**)&ppzval ) == FAILURE )
		{
			throwExceptionWithMessage( 1, "AffineMatrix should contain keys: sx, rx, ry, sy, tx and ty.", 1 TSRMLS_CC);
			RETURN_FALSE;
		}
		else if( Z_TYPE_PP( ppzval ) != IS_DOUBLE || Z_TYPE_PP( ppzval ) != IS_LONG )
		{
			throwExceptionWithMessage( 1, "AffineMatrix values should be ints or floats.", 1 TSRMLS_CC);
			RETURN_FALSE;
		}
		else
		{
			if( Z_TYPE_PP( ppzval ) == IS_LONG )
			{
				value = (double) Z_LVAL_PP( ppzval );
			}
			else
			{
				value = Z_DVAL_PP( ppzval );
			}

			if ( strcmp( matrixElements[i], "sx" ) == 0 )
			{
				pmatrix->sx = value;
			}
			else if ( strcmp( matrixElements[i], "rx" ) == 0 )
			{
				pmatrix->rx = value;
			}
			else if ( strcmp( matrixElements[i], "ry" ) == 0 )
			{
				pmatrix->ry = value;
			}
			else if ( strcmp( matrixElements[i], "sy" ) == 0 )
			{
				pmatrix->sy = value;
			}
			else if ( strcmp( matrixElements[i], "tx" ) == 0 )
			{
				pmatrix->tx = value;
			}
			else if ( strcmp( matrixElements[i], "ty" ) == 0 )
			{
				pmatrix->ty = value;
			}
			else
			{
				throwExceptionWithMessage( 1, "Unkown key in AffineMatrix.", 1 TSRMLS_CC);
				RETURN_FALSE;
			}
		}
	}
	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawAffine( internd->drawing_wand, pmatrix );
	efree( pmatrix );

	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, composite)
{
	php_imagickdraw_object *internd;
	php_imagick_object *intern;
	zval *object, *magickObj;
	long compose;
	double x, y, width, height;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 6 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "lddddO", &compose, &x, &y, &width, &height, &magickObj, php_imagick_sc_entry ) == FAILURE )
	{
		return;
	}

	intern = (php_imagick_object *) zend_object_store_get_object(magickObj TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	status = DrawComposite( internd->drawing_wand, compose, x, y, width, height, intern->magick_wand );

	if ( status == MagickFalse )
	{
		throwImagickDrawException( internd->drawing_wand, 2 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, color)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x, y;
	long paintMethod;


	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ddl", &x, &y, &paintMethod ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawColor( internd->drawing_wand, x, y, paintMethod );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, comment)
{
	zval *object;
	php_imagickdraw_object *internd;
	char *comment;
	int commentLen;


	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &comment, &commentLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawComment( internd->drawing_wand, comment );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, getclippath)
{
	zval *object;
	php_imagickdraw_object *internd;
	char *clipPath;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	clipPath = DrawGetClipPath( internd->drawing_wand );

	ZVAL_STRING( return_value, clipPath, 1 );
	IMAGICK_FREE_MEMORY( char *, clipPath );

	return;
}

PHP_METHOD(imagickdraw, getcliprule)
{
	zval *object;
	php_imagickdraw_object *internd;
	long clipRule;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	clipRule = DrawGetClipRule( internd->drawing_wand );

	RETVAL_LONG( clipRule );
}

PHP_METHOD(imagickdraw, getclipunits)
{
	zval *object;
	php_imagickdraw_object *internd;
	long units;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	units = DrawGetClipUnits( internd->drawing_wand );

	RETVAL_LONG( units );
}

PHP_METHOD(imagickdraw, getfillcolor)
{
	php_imagickpixel_object *internp;
	php_imagickdraw_object *internd;
	zval *object;
	PixelWand *tmpWand;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	object_init_ex( return_value, php_imagickpixel_sc_entry );

	tmpWand = NewPixelWand();
	DrawGetFillColor( internd->drawing_wand, tmpWand );

	internp = (php_imagickpixel_object *) zend_object_store_get_object(return_value TSRMLS_CC);
	internp->pixel_wand = tmpWand;

	return;
}

PHP_METHOD(imagickdraw, getfillopacity)
{
	zval *object;
	php_imagickdraw_object *internd;
	double opacity;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	opacity = DrawGetFillOpacity( internd->drawing_wand );

	RETVAL_DOUBLE( opacity );
}

PHP_METHOD(imagickdraw, getfillrule)
{
	zval *object;
	php_imagickdraw_object *internd;
	long fillRule;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	fillRule = DrawGetFillRule( internd->drawing_wand );

	RETVAL_LONG( fillRule );
}

PHP_METHOD(imagickdraw, getgravity)
{
	zval *object;
	php_imagickdraw_object *internd;
	long gravity;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	gravity = DrawGetGravity( internd->drawing_wand );

	RETVAL_LONG( gravity );
}

PHP_METHOD(imagickdraw, getstrokeantialias)
{
	zval *object;
	php_imagickdraw_object *internd;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	status = DrawGetStrokeAntialias( internd->drawing_wand );

	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}
	else
	{
		RETURN_TRUE;
	}
}

PHP_METHOD(imagickdraw, getstrokecolor)
{
	php_imagickpixel_object *internp;
	php_imagickdraw_object *internd;
	zval *object;
	PixelWand *tmpWand;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	object_init_ex( return_value, php_imagickpixel_sc_entry );

	tmpWand = NewPixelWand();
	DrawGetStrokeColor( internd->drawing_wand, tmpWand );

	internp = (php_imagickpixel_object *) zend_object_store_get_object(return_value TSRMLS_CC);
	internp->pixel_wand = tmpWand;

	return;
}

PHP_METHOD(imagickdraw, getstrokedasharray)
{
	zval *object;
	php_imagickdraw_object *internd;
	double *strokeArray;
	unsigned long numElements, i;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );

	strokeArray = DrawGetStrokeDashArray( internd->drawing_wand, &numElements );
	array_init( return_value );

	for ( i = 0; i < numElements ; i++ )
	{
		add_next_index_double( return_value, strokeArray[i] );
	}
	IMAGICK_FREE_MEMORY( double *, strokeArray );
	return;
}

PHP_METHOD(imagickdraw, setstrokedasharray)
{
	zval *zArray;
	double *dArray;
	long elements;
	zval *object;
	php_imagickdraw_object *internd;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "a", &zArray ) == FAILURE )
	{
		return;
	}

	dArray = getDoublesFromZval( zArray, &elements TSRMLS_CC );

	if ( dArray == (double *)NULL )
	{
		throwExceptionWithMessage( 2, "Can't read array.", 2 TSRMLS_CC );
		RETURN_FALSE;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawSetStrokeDashArray( internd->drawing_wand, elements, dArray );

	efree( dArray );

	RETURN_TRUE;

}

PHP_METHOD(imagickdraw, getstrokedashoffset)
{
	zval *object;
	php_imagickdraw_object *internd;
	double offset;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	offset = DrawGetStrokeDashOffset( internd->drawing_wand );

	RETVAL_DOUBLE( offset );
}

PHP_METHOD(imagickdraw, getstrokelinecap)
{
	zval *object;
	php_imagickdraw_object *internd;
	long lineCap;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	lineCap = DrawGetStrokeLineCap( internd->drawing_wand );

	RETVAL_LONG( lineCap );
}

PHP_METHOD(imagickdraw, getstrokelinejoin)
{
	zval *object;
	php_imagickdraw_object *internd;
	long lineJoin;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	lineJoin = DrawGetStrokeLineJoin( internd->drawing_wand );

	RETVAL_LONG( lineJoin );
}

PHP_METHOD(imagickdraw, getstrokemiterlimit)
{
	zval *object;
	php_imagickdraw_object *internd;
	unsigned long miterLimit;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	miterLimit = DrawGetStrokeMiterLimit( internd->drawing_wand );

	RETVAL_LONG( miterLimit );
}

PHP_METHOD(imagickdraw, getstrokeopacity)
{
	zval *object;
	php_imagickdraw_object *internd;
	double opacity;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	opacity = DrawGetStrokeOpacity( internd->drawing_wand );

	RETVAL_DOUBLE( opacity );
}

PHP_METHOD(imagickdraw, getstrokewidth)
{
	zval *object;
	php_imagickdraw_object *internd;
	double width;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	width = DrawGetStrokeWidth( internd->drawing_wand );

	RETVAL_DOUBLE( width );
}

PHP_METHOD(imagickdraw, gettextalignment)
{
	zval *object;
	php_imagickdraw_object *internd;
	long alignType;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	alignType = DrawGetTextAlignment( internd->drawing_wand );

	RETVAL_LONG( alignType );
}

PHP_METHOD(imagickdraw, gettextantialias)
{
	zval *object;
	php_imagickdraw_object *internd;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	status = DrawGetTextAntialias( internd->drawing_wand );

	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}
	else
	{
		RETURN_TRUE;
	}
}

PHP_METHOD(imagickdraw, getvectorgraphics)
{
	zval *object;
	php_imagickdraw_object *internd;
	char *vector;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	vector = DrawGetVectorGraphics( internd->drawing_wand );

	ZVAL_STRING( return_value, vector, 1 );
	IMAGICK_FREE_MEMORY( char *, vector );

	return;
}

PHP_METHOD(imagickdraw, gettextundercolor)
{
	php_imagickpixel_object *internp;
	php_imagickdraw_object *internd;
	zval *object;
	PixelWand *tmpWand;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	object_init_ex( return_value, php_imagickpixel_sc_entry );

	tmpWand = NewPixelWand();
	DrawGetTextUnderColor( internd->drawing_wand, tmpWand );

	internp = (php_imagickpixel_object *) zend_object_store_get_object(return_value TSRMLS_CC);
	internp->pixel_wand = tmpWand;

	return;
}

PHP_METHOD(imagickdraw, pathclose)
{
	zval *object;
	php_imagickdraw_object *internd;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	DrawPathClose( internd->drawing_wand );
	RETURN_TRUE;
}


PHP_METHOD(imagickdraw, pathcurvetoabsolute)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x1, y1, x2, y2, x, y;


	if ( ZEND_NUM_ARGS() != 6 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddddd", &x1, &y1, &x2, &y2, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathCurveToAbsolute( internd->drawing_wand, x1, y1, x2, y2, x, y );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathcurvetorelative)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x1, y1, x2, y2, x, y;


	if ( ZEND_NUM_ARGS() != 6 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddddd", &x1, &y1, &x2, &y2, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathCurveToRelative( internd->drawing_wand, x1, y1, x2, y2, x, y );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathcurvetoquadraticbezierabsolute)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x1, y1, x, y;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddd", &x1, &y1, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathCurveToQuadraticBezierAbsolute( internd->drawing_wand, x1, y1, x, y );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathcurvetoquadraticbezierrelative)
{

	zval *object;
	php_imagickdraw_object *internd;
	double x1, y1, x, y;

	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddd", &x1, &y1, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathCurveToQuadraticBezierRelative( internd->drawing_wand, x1, y1, x, y );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathcurvetoquadraticbeziersmoothabsolute)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x, y;


	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathCurveToQuadraticBezierSmoothAbsolute( internd->drawing_wand, x, y );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathcurvetoquadraticbeziersmoothrelative)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x, y;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathCurveToQuadraticBezierSmoothRelative( internd->drawing_wand, x, y );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathcurvetosmoothabsolute)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x1, y1, x, y;


	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddd", &x1, &y1, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathCurveToSmoothAbsolute( internd->drawing_wand, x1, y1, x, y );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathcurvetosmoothrelative)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x1, y1, x, y;


	if ( ZEND_NUM_ARGS() != 4 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddd", &x1, &y1, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathCurveToSmoothRelative( internd->drawing_wand, x1, y1, x, y );
	RETURN_TRUE;
}



PHP_METHOD(imagickdraw, pathellipticarcabsolute)
{
	zval *object;
	php_imagickdraw_object *internd;
	double rx, ry, xAxisRotation, x, y;
	zend_bool largeArc, sweep;

	if ( ZEND_NUM_ARGS() != 7 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddbbdd", &rx, &ry, &xAxisRotation, &largeArc, &sweep, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);
	DrawPathEllipticArcAbsolute( internd->drawing_wand, rx, ry, xAxisRotation, largeArc, sweep, x, y );

	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathellipticarcrelative)
{

	zval *object;
	php_imagickdraw_object *internd;
	double rx, ry, xAxisRotation, x, y;
	zend_bool largeArc, sweep;

	if ( ZEND_NUM_ARGS() != 7 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dddbbdd", &rx, &ry, &xAxisRotation, &largeArc, &sweep, &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);
	DrawPathEllipticArcRelative( internd->drawing_wand, rx, ry, xAxisRotation, largeArc, sweep, x, y );

	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathfinish)
{
	zval *object;
	php_imagickdraw_object *internd;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	DrawPathFinish( internd->drawing_wand );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathlinetoabsolute)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x, y;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathLineToAbsolute( internd->drawing_wand, x, y );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathlinetorelative)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x, y;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathLineToRelative( internd->drawing_wand, x, y );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathlinetohorizontalabsolute)
{
	zval *object;
	php_imagickdraw_object *internd;
	double y;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d",  &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathLineToHorizontalAbsolute( internd->drawing_wand, y );
	RETURN_TRUE;

}

PHP_METHOD(imagickdraw, pathlinetohorizontalrelative)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &x ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathLineToHorizontalRelative( internd->drawing_wand, x );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathlinetoverticalabsolute)
{
	zval *object;
	php_imagickdraw_object *internd;
	double y;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathLineToVerticalAbsolute( internd->drawing_wand, y );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathlinetoverticalrelative)
{
	zval *object;
	php_imagickdraw_object *internd;
	double y;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathLineToVerticalRelative( internd->drawing_wand, y );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathmovetoabsolute)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x, y;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathMoveToAbsolute( internd->drawing_wand, x, y );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathmovetorelative)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x, y;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPathMoveToRelative( internd->drawing_wand, x, y );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pathstart)
{
	zval *object;
	php_imagickdraw_object *internd;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	DrawPathStart( internd->drawing_wand );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, polyline)
{

	zval *coordinateArray, *object;
	php_imagickdraw_object *internd;
	PointInfo *coordinates;
	int numElements = 0;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "a", &coordinateArray ) == FAILURE )
	{
		return;
	}

	coordinates = getPointInfoFromZval( coordinateArray, &numElements TSRMLS_CC );

	if ( coordinates == (PointInfo *)NULL )
	{
		efree( coordinates );
		throwExceptionWithMessage( 1, "Unable to read coordinate array.", 1 TSRMLS_CC );
		RETURN_FALSE;
	}

	object = getThis();
	internd = (php_imagickdraw_object *)zend_object_store_get_object(object TSRMLS_CC);
	DrawPolyline( internd->drawing_wand, numElements, coordinates );

	efree( coordinates );
	RETURN_TRUE;

}

PHP_METHOD(imagickdraw, popclippath)
{
	zval *object;
	php_imagickdraw_object *internd;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	DrawPopClipPath( internd->drawing_wand );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, popdefs)
{
	zval *object;
	php_imagickdraw_object *internd;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	DrawPopDefs( internd->drawing_wand );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, poppattern)
{
	zval *object;
	php_imagickdraw_object *internd;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	status = DrawPopPattern( internd->drawing_wand );

	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}
	else
	{
		RETURN_TRUE;
	}
}

PHP_METHOD(imagickdraw, pushclippath)
{
	zval *object;
	php_imagickdraw_object *internd;
	char *clipMask;
	int clipMaskLen;


	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &clipMask, &clipMaskLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPushClipPath( internd->drawing_wand, clipMask );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pushdefs)
{
	zval *object;
	php_imagickdraw_object *internd;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	DrawPushDefs( internd->drawing_wand );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, pushpattern)
{
	zval *object;
	php_imagickdraw_object *internd;
	char *patternId;
	int patternIdLen;
	double x, y, width, height;


	if ( ZEND_NUM_ARGS() != 5 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "sdddd", &patternId, &patternIdLen, &x, &y, &width, &height ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawPushPattern( internd->drawing_wand, patternId, x, y, width, height );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, render)
{
	zval *object;
	php_imagickdraw_object *internd;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	status = DrawRender( internd->drawing_wand );

	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}
	else
	{
		RETURN_TRUE;
	}
}

PHP_METHOD(imagickdraw, resetvectorgraphics)
{
	zval *object;
	php_imagickdraw_object *internd;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	DrawResetVectorGraphics( internd->drawing_wand );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, rotate)
{
	zval *object;
	php_imagickdraw_object *internd;
	double degrees;


	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &degrees ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawRotate( internd->drawing_wand, degrees );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, scale)
{
	zval *object;
	php_imagickdraw_object *internd;
	double x, y;


	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "dd", &x, &y ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawScale( internd->drawing_wand, x, y );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, setclippath)
{
	zval *object;
	php_imagickdraw_object *internd;
	char *clipMask;
	int clipMaskLen;
	MagickBooleanType status;


	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &clipMask, &clipMaskLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	status = DrawSetClipPath( internd->drawing_wand, clipMask );

	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}
	else
	{
		RETURN_TRUE;
	}
}


PHP_METHOD(imagickdraw, setcliprule)
{
	zval *object;
	php_imagickdraw_object *internd;
	long fillRule;


	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &fillRule ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawSetClipRule( internd->drawing_wand, fillRule );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, setclipunits)
{
	zval *object;
	php_imagickdraw_object *internd;
	long units;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &units ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawSetClipUnits( internd->drawing_wand, units );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, setfillopacity)
{
	zval *object;
	php_imagickdraw_object *internd;
	double fillOpacity;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &fillOpacity ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawSetFillOpacity( internd->drawing_wand, fillOpacity );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, setfillpatternurl)
{
	zval *object;
	php_imagickdraw_object *internd;
	char *url;
	int urlLen;
	MagickBooleanType status;


	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &url, &urlLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	status = DrawSetFillPatternURL( internd->drawing_wand, url );

	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}
	else
	{
		RETURN_TRUE;
	}
}

PHP_METHOD(imagickdraw, setfillrule)
{
	zval *object;
	php_imagickdraw_object *internd;
	long fillRule;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &fillRule ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawSetFillRule( internd->drawing_wand, fillRule );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, setgravity)
{
	zval *object;
	php_imagickdraw_object *internd;
	long gravity;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &gravity ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawSetGravity( internd->drawing_wand, gravity );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, setstrokepatternurl)
{
	zval *object;
	php_imagickdraw_object *internd;
	char *url;
	int urlLen;
	MagickBooleanType status;


	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &url, &urlLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	status = DrawSetStrokePatternURL( internd->drawing_wand, url );

	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}
	else
	{
		RETURN_TRUE;
	}
}

PHP_METHOD(imagickdraw, setstrokedashoffset)
{
	zval *object;
	php_imagickdraw_object *internd;
	double offset;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &offset ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawSetStrokeDashOffset( internd->drawing_wand, offset );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, setstrokelinecap)
{
	zval *object;
	php_imagickdraw_object *internd;
	long lineCap;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &lineCap ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawSetStrokeLineCap( internd->drawing_wand, lineCap );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, setstrokelinejoin)
{
	zval *object;
	php_imagickdraw_object *internd;
	long lineJoin;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &lineJoin ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawSetStrokeLineJoin( internd->drawing_wand, lineJoin );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, setstrokemiterlimit)
{
	zval *object;
	php_imagickdraw_object *internd;
	long miterLimit;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &miterLimit ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawSetStrokeMiterLimit( internd->drawing_wand, miterLimit );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, setstrokeopacity)
{
	zval *object;
	php_imagickdraw_object *internd;
	double opacity;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &opacity ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	DrawSetStrokeOpacity( internd->drawing_wand, opacity );
	RETURN_TRUE;
}

PHP_METHOD(imagickdraw, setvectorgraphics)
{

	zval *object;
	php_imagickdraw_object *internd;
	char *vector;
	int vectorLen;
	MagickBooleanType status;


	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &vector, &vectorLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internd = (php_imagickdraw_object *) zend_object_store_get_object(object TSRMLS_CC);

	status = DrawSetVectorGraphics( internd->drawing_wand, vector );

	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}
	else
	{
		RETURN_TRUE;
	}
}

PHP_METHOD(imagickdraw, popdrawingwand)
{
	zval *object;
	php_imagickdraw_object *internd;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	status = PopDrawingWand( internd->drawing_wand );

	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}
	else
	{
		RETURN_TRUE;
	}
}

PHP_METHOD(imagickdraw, pushdrawingwand)
{
	zval *object;
	php_imagickdraw_object *internd;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickdraw_object *, internd );
	status = PushDrawingWand( internd->drawing_wand );

	if ( status == MagickFalse )
	{
		RETURN_FALSE;
	}
	else
	{
		RETURN_TRUE;
	}
}

/* END OF DRAWINGWAND METHODS */

PHP_METHOD(imagickpixeliterator, __construct)
{
	RETURN_TRUE;
}


PHP_METHOD(imagickpixeliterator, resetiterator)
{
	zval *object;
	php_imagickpixeliterator_object *internpix;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixeliterator_object *, internpix );

	if ( internpix->instanciated_correctly < 1 )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	if ( !IsPixelIterator( internpix->pixel_iterator ) )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	PixelResetIterator( internpix->pixel_iterator );

	RETURN_TRUE;
}


PHP_METHOD(imagickpixeliterator, synciterator)
{
	zval *object;
	php_imagickpixeliterator_object *internpix;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixeliterator_object *, internpix );

	if ( internpix->instanciated_correctly < 1 )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	if ( !IsPixelIterator( internpix->pixel_iterator ) )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	PixelSyncIterator( internpix->pixel_iterator );

	if ( status == MagickFalse )
	{
		throwExceptionWithMessage( 3, "Invalid MagickWand passed.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagickpixeliterator, setiteratorfirstrow)
{
	zval *object;
	php_imagickpixeliterator_object *internpix;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixeliterator_object *, internpix );

	if ( internpix->instanciated_correctly < 1 )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	if ( !IsPixelIterator( internpix->pixel_iterator ) )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	PixelSetFirstIteratorRow( internpix->pixel_iterator );

	RETURN_TRUE;
}


PHP_METHOD(imagickpixeliterator, setiteratorlastrow)
{
	zval *object;
	php_imagickpixeliterator_object *internpix;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixeliterator_object *, internpix );

	if ( internpix->instanciated_correctly != 1 )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	if ( !IsPixelIterator( internpix->pixel_iterator ) )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	PixelSetLastIteratorRow( internpix->pixel_iterator );

	RETURN_TRUE;
}


PHP_METHOD(imagickpixeliterator, newpixeliterator)
{

	zval *magickObject;
	zval *object;
	php_imagickpixeliterator_object *internpix;
	php_imagick_object *intern;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "O", &magickObject, php_imagick_sc_entry ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internpix = (php_imagickpixeliterator_object *)zend_object_store_get_object(object TSRMLS_CC);
	intern = (php_imagick_object *)zend_object_store_get_object(magickObject TSRMLS_CC);
	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = IsMagickWand( intern->magick_wand );

	if ( status == MagickFalse )
	{
		throwExceptionWithMessage( 3, "Invalid MagickWand passed.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	internpix->pixel_iterator = NewPixelIterator( intern->magick_wand );
	internpix->iterator_type = 1;

	if ( !IsPixelIterator( internpix->pixel_iterator ) )
	{
		throwExceptionWithMessage( 3, "Can not allocate PixelIterator", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	internpix->instanciated_correctly = 1;

	RETURN_TRUE;
}


PHP_METHOD(imagickpixeliterator, newpixelregioniterator)
{
	zval *magickObject;
	zval *object;
	php_imagickpixeliterator_object *internpix;
	php_imagick_object *intern;
	MagickBooleanType status;
	zval *x, *y, *columns, *rows;

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "Ozzzz", &magickObject, php_imagick_sc_entry, &x, &y, &columns, &rows ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internpix = (php_imagickpixeliterator_object *)zend_object_store_get_object(object TSRMLS_CC);
	intern = (php_imagick_object *)zend_object_store_get_object(magickObject TSRMLS_CC);

	IMAGICK_CHECK_NOT_EMPTY( intern->magick_wand, 1, 1 );

	status = IsMagickWand( intern->magick_wand );

	if ( status == MagickFalse )
	{
		throwExceptionWithMessage( 3, "Invalid MagickWand passed.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	internpix->pixel_iterator = NewPixelRegionIterator( intern->magick_wand, Z_LVAL_P( x ), Z_LVAL_P(y), Z_LVAL_P(columns), Z_LVAL_P(rows) );
	internpix->iterator_type = 2;

	if ( !IsPixelIterator( internpix->pixel_iterator ) )
	{
		throwExceptionWithMessage( 3, "Can not allocate PixelIterator", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	internpix->instanciated_correctly = 1;

	RETURN_TRUE;
}


PHP_METHOD(imagickpixeliterator, getiteratorrow)
{
	zval *object;
	php_imagickpixeliterator_object *internpix;
	MagickBooleanType status;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixeliterator_object *, internpix );

	if ( internpix->instanciated_correctly < 1 )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
	}

	if ( !IsPixelIterator( internpix->pixel_iterator ) )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
	}

	status = PixelGetIteratorRow( internpix->pixel_iterator );

	ZVAL_LONG( return_value, (long) status );

}


PHP_METHOD(imagickpixeliterator, setiteratorrow)
{
	zval *object;
	php_imagickpixeliterator_object *internpix;
	MagickBooleanType status;
	long row;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &row ) == FAILURE )
	{
		return;
	}
	object = getThis();
	internpix = (php_imagickpixeliterator_object *)zend_object_store_get_object(object TSRMLS_CC);

	if ( internpix->instanciated_correctly < 1 )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	if ( !IsPixelIterator( internpix->pixel_iterator ) )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	if ( internpix->iterator_type == 2 )
	{
		throwExceptionWithMessage( 3, "Can't set row in RegionPixelIterator.", 3 TSRMLS_CC);
	}

	status = PixelSetIteratorRow( internpix->pixel_iterator, row );

	if ( status == MagickFalse)
	{
		throwImagickPixelIteratorException( internpix->pixel_iterator, 3 TSRMLS_CC );
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagickpixeliterator, getpreviousiteratorrow)
{
	zval *object;
	php_imagickpixeliterator_object *internpix;
	php_imagickpixel_object *internp;
	PixelWand **wandArray;
	unsigned long numWands;
	int i;
	zval *tmpPixelWand;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixeliterator_object *, internpix );

	if ( internpix->instanciated_correctly < 1 )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	if ( !IsPixelIterator( internpix->pixel_iterator ) )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	wandArray = PixelGetPreviousIteratorRow( internpix->pixel_iterator, &numWands );
	array_init( return_value );

	for (i = 0; i < numWands; i++ )
	{
		if ( IsPixelWand ( wandArray[i] ) )
		{
			MAKE_STD_ZVAL( tmpPixelWand );
			object_init_ex( tmpPixelWand, php_imagickpixel_sc_entry );
			internp = (php_imagickpixel_object *)zend_object_store_get_object(tmpPixelWand TSRMLS_CC);
			internp->pixel_wand = wandArray[i];
			internp->initialized_via_iterator = 1;
			add_next_index_zval( return_value, tmpPixelWand );
		}
	}

	return;
}


PHP_METHOD(imagickpixeliterator, getcurrentiteratorrow)
{
	zval *object;
	php_imagickpixeliterator_object *internpix;
	php_imagickpixel_object *internp;
	PixelWand **wandArray;
	long numWands;
	int i;
	zval *tmpPixelWand;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixeliterator_object *, internpix );

	if ( internpix->instanciated_correctly < 1 )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	if ( !IsPixelIterator( internpix->pixel_iterator ) )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	wandArray = (PixelWand **)PixelGetCurrentIteratorRow( internpix->pixel_iterator, &numWands );
	array_init( return_value );

	for (i = 0; i < numWands; i++ )
	{
		if ( IsPixelWand ( wandArray[i] ) )
		{
			MAKE_STD_ZVAL( tmpPixelWand );
			object_init_ex( tmpPixelWand, php_imagickpixel_sc_entry );
			internp = (php_imagickpixel_object *)zend_object_store_get_object(tmpPixelWand TSRMLS_CC);
			internp->pixel_wand = wandArray[i];
			internp->initialized_via_iterator = 1;
			add_next_index_zval( return_value, tmpPixelWand );
		}
	}
	return;
}


PHP_METHOD(imagickpixeliterator, getnextiteratorrow)
{
	zval *object;
	php_imagickpixeliterator_object *internpix;
	php_imagickpixel_object *internp;
	PixelWand **wandArray;
	unsigned long numWands;
	int i;
	zval *tmpPixelWand;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixeliterator_object *, internpix );

	if ( internpix->instanciated_correctly < 1 )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	if ( !IsPixelIterator( internpix->pixel_iterator ) )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	wandArray = PixelGetNextIteratorRow( internpix->pixel_iterator, &numWands );
	array_init( return_value );

	for (i = 0; i < numWands; i++ )
	{
		if ( IsPixelWand ( wandArray[i] ) )
		{
			MAKE_STD_ZVAL( tmpPixelWand );
			object_init_ex( tmpPixelWand, php_imagickpixel_sc_entry );
			internp = (php_imagickpixel_object *)zend_object_store_get_object(tmpPixelWand TSRMLS_CC);
			internp->pixel_wand = wandArray[i];
			internp->initialized_via_iterator = 1;
			add_next_index_zval( return_value, tmpPixelWand );
		}
	}

	return;
}

PHP_METHOD(imagickpixeliterator, destroy)
{
	zval *object;
	php_imagickpixeliterator_object *internpix;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixeliterator_object *, internpix );

	if ( internpix->instanciated_correctly < 1 )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	if ( !IsPixelIterator( internpix->pixel_iterator ) )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	internpix->pixel_iterator = DestroyPixelIterator( internpix->pixel_iterator );
	RETURN_TRUE;
}

PHP_METHOD(imagickpixeliterator, clear)
{
	zval *object;
	php_imagickpixeliterator_object *internpix;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixeliterator_object *, internpix );

	if ( internpix->instanciated_correctly < 1 )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	if ( !IsPixelIterator( internpix->pixel_iterator ) )
	{
		throwExceptionWithMessage( 3, "PixelIterator is not initialized correctly.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	ClearPixelIterator( internpix->pixel_iterator );
	RETURN_TRUE;
}

/* END OF PIXELITERATOR */

PHP_METHOD(imagickpixel, __construct)
{
	zval *object;
	php_imagickpixel_object *internp;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	internp->pixel_wand = NewPixelWand();
	RETURN_TRUE;
}


PHP_METHOD(imagickpixel, setalpha)
{
	zval *object;
	double opacity;
	php_imagickpixel_object *internp;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "d", &opacity ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internp = (php_imagickpixel_object *)zend_object_store_get_object(object TSRMLS_CC);

	PixelSetAlpha( internp->pixel_wand, opacity );
	RETURN_TRUE;
}


PHP_METHOD(imagickpixel, getalpha)
{
	zval *object;
	double alpha;
	php_imagickpixel_object *internp;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	alpha = PixelGetAlpha( internp->pixel_wand );
	ZVAL_DOUBLE( return_value, alpha );

	return;
}


PHP_METHOD(imagickpixel, setcolor)
{
	char *colorName;
	int colorNameLen;
	zval *object;
	php_imagickpixel_object *internp;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 1 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &colorName, &colorNameLen ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internp = (php_imagickpixel_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = PixelSetColor( internp->pixel_wand , colorName );

	if( status == MagickFalse )
	{
		throwImagickPixelException( internp->pixel_wand, 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagickpixel, clear)
{
	zval *object;
	php_imagickpixel_object *internp;
	MagickBooleanType status;

	object = getThis();
	internp = (php_imagickpixel_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = IsPixelWand( internp->pixel_wand );

	if ( status == MagickFalse )
	{
		throwExceptionWithMessage( 3, "PixelWand is not allocated.", 3 TSRMLS_CC);
		RETURN_FALSE;
	}

	ClearPixelWand( internp->pixel_wand );
	RETURN_TRUE;
}


PHP_METHOD(imagickpixel, destroy)
{
	zval *object;
	php_imagickpixel_object *internp;
	MagickBooleanType status;

	object = getThis();
	internp = (php_imagickpixel_object *)zend_object_store_get_object(object TSRMLS_CC);

	status = IsPixelWand( internp->pixel_wand );

	if ( status == MagickFalse )
	{
		throwExceptionWithMessage( 4, "PixelWand is not allocated.", 4 TSRMLS_CC);
		RETURN_FALSE;
	}

	ClearPixelWand( internp->pixel_wand );
	internp->pixel_wand = DestroyPixelWand( internp->pixel_wand );
	RETURN_TRUE;
}


PHP_METHOD(imagickpixel, issimilar)
{
	zval *object, *objvar;
	double fuzz;
	php_imagickpixel_object *internp, *internp_second;
	MagickBooleanType status;

	if ( ZEND_NUM_ARGS() != 2 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "Od", &objvar, php_imagickpixel_sc_entry, &fuzz ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internp = (php_imagickpixel_object *)zend_object_store_get_object(object TSRMLS_CC);
	internp_second = (php_imagickpixel_object *)zend_object_store_get_object(objvar TSRMLS_CC);

	status = IsPixelWandSimilar( internp->pixel_wand, internp_second->pixel_wand, fuzz );

	if( status == MagickFalse )
	{
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(imagickpixel, getblack)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	color = PixelGetBlack( internp->pixel_wand );
	RETVAL_DOUBLE( color );
}


PHP_METHOD(imagickpixel, setblack)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICKPIXEL_INITIALIZE_SET_COLOR( object, internp, color );

	PixelSetBlack( internp->pixel_wand, color );
	RETURN_TRUE;
}


PHP_METHOD(imagickpixel, getblue)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	color = PixelGetBlue( internp->pixel_wand );
	RETVAL_DOUBLE( color );
}


PHP_METHOD(imagickpixel, setblue)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICKPIXEL_INITIALIZE_SET_COLOR( object, internp, color );

	PixelSetBlue( internp->pixel_wand, color );
	RETURN_TRUE;
}


PHP_METHOD(imagickpixel, getcyan)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	color = PixelGetCyan( internp->pixel_wand );
	RETVAL_DOUBLE( color );
}


PHP_METHOD(imagickpixel, setcyan)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICKPIXEL_INITIALIZE_SET_COLOR( object, internp, color );

	PixelSetCyan( internp->pixel_wand, color );
	RETURN_TRUE;

}


PHP_METHOD(imagickpixel, getgreen)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	color = PixelGetGreen( internp->pixel_wand );
	RETVAL_DOUBLE( color );
}


PHP_METHOD(imagickpixel, setgreen)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICKPIXEL_INITIALIZE_SET_COLOR( object, internp, color );

	PixelSetGreen( internp->pixel_wand, color );
	RETURN_TRUE;

}


PHP_METHOD(imagickpixel, getred)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	color = PixelGetRed( internp->pixel_wand );
	RETVAL_DOUBLE( color );
}


PHP_METHOD(imagickpixel, setred)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICKPIXEL_INITIALIZE_SET_COLOR( object, internp, color );

	PixelSetRed( internp->pixel_wand, color );
	RETURN_TRUE;
}


PHP_METHOD(imagickpixel, getyellow)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	color = PixelGetYellow( internp->pixel_wand );
	RETVAL_DOUBLE( color );
}


PHP_METHOD(imagickpixel, setyellow)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICKPIXEL_INITIALIZE_SET_COLOR( object, internp, color );

	PixelSetYellow( internp->pixel_wand, color );
	RETURN_TRUE;
}


PHP_METHOD(imagickpixel, getmagenta)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	color = PixelGetMagenta( internp->pixel_wand );
	RETVAL_DOUBLE( color );
}


PHP_METHOD(imagickpixel, setmagenta)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICKPIXEL_INITIALIZE_SET_COLOR( object, internp, color );

	PixelSetMagenta( internp->pixel_wand, color );
	RETURN_TRUE;
}


PHP_METHOD(imagickpixel, getopacity)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	color = PixelGetOpacity( internp->pixel_wand );
	RETVAL_DOUBLE( color );
}


PHP_METHOD(imagickpixel, setopacity)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICKPIXEL_INITIALIZE_SET_COLOR( object, internp, color );

	PixelSetOpacity( internp->pixel_wand, color );
	RETURN_TRUE;
}


PHP_METHOD(imagickpixel, getfuzz)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	color = PixelGetFuzz( internp->pixel_wand );
	RETVAL_DOUBLE( color );
}


PHP_METHOD(imagickpixel, setfuzz)
{
	zval *object;
	php_imagickpixel_object *internp;
	double color;

	IMAGICKPIXEL_INITIALIZE_SET_COLOR( object, internp, color );

	PixelSetFuzz( internp->pixel_wand, color );
	RETURN_TRUE;
}


PHP_METHOD(imagickpixel, gethsl)
{
	zval *object;
	php_imagickpixel_object *internp;
	double hue, saturation, luminosity;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	PixelGetHSL( internp->pixel_wand, &hue, &saturation, &luminosity );

	array_init( return_value );
	add_assoc_double( return_value, "hue", hue );
	add_assoc_double( return_value, "saturation", saturation );
	add_assoc_double( return_value, "luminosity", luminosity );
	return;
}


PHP_METHOD(imagickpixel, sethsl)
{
	zval *object;
	php_imagickpixel_object *internp;
	double hue, saturation, luminosity;

	if ( ZEND_NUM_ARGS() != 3 )
	{
		ZEND_WRONG_PARAM_COUNT();
	}

	/* Parse parameters given to function */
	if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ddd", &hue, &saturation, &luminosity ) == FAILURE )
	{
		return;
	}

	object = getThis();
	internp = (php_imagickpixel_object *)zend_object_store_get_object(object TSRMLS_CC);

	PixelSetHSL( internp->pixel_wand, hue, saturation, luminosity );
	RETURN_TRUE;
}


PHP_METHOD(imagickpixel, getcolorasstring)
{

	zval *object;
	php_imagickpixel_object *internp;
	char *colorString;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	colorString = PixelGetColorAsString( internp->pixel_wand );
	ZVAL_STRING( return_value, colorString, 1 );
	IMAGICK_FREE_MEMORY( char *, colorString );
	return;

}


PHP_METHOD(imagickpixel, getcolorasnormalizedstring)
{

	zval *object;
	php_imagickpixel_object *internp;
	char *colorString;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	colorString = (char *)PixelGetColorAsNormalizedString( internp->pixel_wand );
	ZVAL_STRING( return_value, colorString, 1 );
	IMAGICK_FREE_MEMORY( char *, colorString );
	return;

}


PHP_METHOD(imagickpixel, getcolorcount)
{
	zval *object;
	php_imagickpixel_object *internp;
	long colorCount;

	IMAGICK_INITIALIZE_ZERO_ARGS( object, php_imagickpixel_object *, internp );

	colorCount = PixelGetColorCount( internp->pixel_wand );
	RETVAL_LONG( colorCount );
}





/* End of pixelwand methods */

void initializeMagickConstants()
{
	TSRMLS_FETCH();

	IMAGICK_REGISTER_CONST_LONG( "COMPOSITE_OVER", OverCompositeOp );
	IMAGICK_REGISTER_CONST_LONG( "COMPOSITE_IN", InCompositeOp );
	IMAGICK_REGISTER_CONST_LONG( "COMPOSITE_OUT", OutCompositeOp );
	IMAGICK_REGISTER_CONST_LONG( "COMPOSITE_ATOP", AtopCompositeOp );
	IMAGICK_REGISTER_CONST_LONG( "COMPOSITE_XOR", XorCompositeOp );
	IMAGICK_REGISTER_CONST_LONG( "COMPOSITE_PLUS", PlusCompositeOp );
	IMAGICK_REGISTER_CONST_LONG( "COMPOSITE_MINUS", MinusCompositeOp );
	IMAGICK_REGISTER_CONST_LONG( "COMPOSITE_ADD", AddCompositeOp );
	IMAGICK_REGISTER_CONST_LONG( "COMPOSITE_SUBTRACT", SubtractCompositeOp );
	IMAGICK_REGISTER_CONST_LONG( "COMPOSITE_DIFFERENCE", DifferenceCompositeOp );
	IMAGICK_REGISTER_CONST_LONG( "COMPOSITE_BUMPMAP", BumpmapCompositeOp);
	IMAGICK_REGISTER_CONST_LONG( "COMPOSITE_COPY", CopyCompositeOp );
	IMAGICK_REGISTER_CONST_LONG( "COMPOSITE_DISPLACE", DisplaceCompositeOp );
	IMAGICK_REGISTER_CONST_LONG( "COMPOSITE_DEFAULT", OverCompositeOp );
	IMAGICK_REGISTER_CONST_LONG( "MONTAGEMODE_FRAME", FrameMode );
	IMAGICK_REGISTER_CONST_LONG( "MONTAGEMODE_UNFRAME", UnframeMode );
	IMAGICK_REGISTER_CONST_LONG( "MONTAGEMODE_CONCATENATE", ConcatenateMode );
	IMAGICK_REGISTER_CONST_LONG( "STYLE_NORMAL", NormalStyle  );
	IMAGICK_REGISTER_CONST_LONG( "STYLE_ITALIC", ItalicStyle  );
	IMAGICK_REGISTER_CONST_LONG( "STYLE_OBLIQUE", ObliqueStyle  );
	IMAGICK_REGISTER_CONST_LONG( "STYLE_ANY", AnyStyle  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_UNDEFINED", UndefinedFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_POINT", PointFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_BOX", BoxFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_TRIANGLE", TriangleFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_HERMITE", HermiteFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_HANNING", HanningFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_HAMMING", HammingFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_BLACKMAN", BlackmanFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_GAUSSIAN", GaussianFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_QUADRATIC", QuadraticFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_CUBIC", CubicFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_CATROM", CatromFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_MITCHELL", MitchellFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_LANCZOS", LanczosFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_BESSEL", BesselFilter  );
	IMAGICK_REGISTER_CONST_LONG( "FILTER_SINC", SincFilter  );
	IMAGICK_REGISTER_CONST_LONG( "IMGTYPE_UNDEFINED", UndefinedType  );
	IMAGICK_REGISTER_CONST_LONG( "IMGTYPE_BILEVEL", BilevelType  );
	IMAGICK_REGISTER_CONST_LONG( "IMGTYPE_GRAYSCALE", GrayscaleType  );
	IMAGICK_REGISTER_CONST_LONG( "IMGTYPE_GRAYSCALEMATTE", GrayscaleMatteType  );
	IMAGICK_REGISTER_CONST_LONG( "IMGTYPE_PALETTE",  PaletteType  );
	IMAGICK_REGISTER_CONST_LONG( "IMGTYPE_PALETTEMATTE", PaletteMatteType  );
	IMAGICK_REGISTER_CONST_LONG( "IMGTYPE_TRUECOLOR", TrueColorType  );
	IMAGICK_REGISTER_CONST_LONG( "IMGTYPE_TRUECOLORMATTE", TrueColorMatteType  );
	IMAGICK_REGISTER_CONST_LONG( "IMGTYPE_COLORSEPARATION", ColorSeparationType  );
	IMAGICK_REGISTER_CONST_LONG( "IMGTYPE_COLORSEPARATIONMATTE", ColorSeparationMatteType  );
	IMAGICK_REGISTER_CONST_LONG( "IMGTYPE_OPTIMIZE", OptimizeType  );
	IMAGICK_REGISTER_CONST_LONG( "RESOLUTION_UNDEFINED", UndefinedResolution  );
	IMAGICK_REGISTER_CONST_LONG( "RESOLUTION_PIXELSPERINCH", PixelsPerInchResolution  );
	IMAGICK_REGISTER_CONST_LONG( "RESOLUTION_PIXELSPERCENTIMETER", PixelsPerCentimeterResolution );
	IMAGICK_REGISTER_CONST_LONG( "COMPRESSION_UNDEFINED", UndefinedCompression);
	IMAGICK_REGISTER_CONST_LONG( "COMPRESSION_NO", NoCompression);
	IMAGICK_REGISTER_CONST_LONG( "COMPRESSION_BZIP", BZipCompression);
	IMAGICK_REGISTER_CONST_LONG( "COMPRESSION_FAX", FaxCompression );
	IMAGICK_REGISTER_CONST_LONG( "COMPRESSION_GROUP4", Group4Compression );
	IMAGICK_REGISTER_CONST_LONG( "COMPRESSION_JPEG", JPEGCompression );
	IMAGICK_REGISTER_CONST_LONG( "COMPRESSION_JPEG2000", JPEG2000Compression );
	IMAGICK_REGISTER_CONST_LONG( "COMPRESSION_LOSSLESSJPEG", LosslessJPEGCompression );
	IMAGICK_REGISTER_CONST_LONG( "COMPRESSION_LZW", LZWCompression );
	IMAGICK_REGISTER_CONST_LONG( "COMPRESSION_RLE", RLECompression );
	IMAGICK_REGISTER_CONST_LONG( "COMPRESSION_ZIP", ZipCompression );
	IMAGICK_REGISTER_CONST_LONG( "PAINT_POINT", PointMethod );
	IMAGICK_REGISTER_CONST_LONG( "PAINT_REPLACE", ReplaceMethod );
	IMAGICK_REGISTER_CONST_LONG( "PAINT_FLOODFILL", FloodfillMethod );
	IMAGICK_REGISTER_CONST_LONG( "PAINT_FILLTOBORDER", FillToBorderMethod );
	IMAGICK_REGISTER_CONST_LONG( "PAINT_RESET", ResetMethod );
	IMAGICK_REGISTER_CONST_LONG( "GRAVITY_NORTHWEST", NorthWestGravity );
	IMAGICK_REGISTER_CONST_LONG( "GRAVITY_NORTH", NorthGravity );
	IMAGICK_REGISTER_CONST_LONG( "GRAVITY_NORTHEAST", NorthEastGravity );
	IMAGICK_REGISTER_CONST_LONG( "GRAVITY_WEST", WestGravity );
	IMAGICK_REGISTER_CONST_LONG( "GRAVITY_CENTER", CenterGravity );
	IMAGICK_REGISTER_CONST_LONG( "GRAVITY_EAST", EastGravity );
	IMAGICK_REGISTER_CONST_LONG( "GRAVITY_SOUTHWEST", SouthWestGravity );
	IMAGICK_REGISTER_CONST_LONG( "GRAVITY_SOUTH", SouthGravity );
	IMAGICK_REGISTER_CONST_LONG( "GRAVITY_SOUTHEAST", SouthEastGravity );
	IMAGICK_REGISTER_CONST_LONG( "STRETCH_NORMAL", NormalStretch );
	IMAGICK_REGISTER_CONST_LONG( "STRETCH_ULTRACONDENSED", UltraCondensedStretch );
	IMAGICK_REGISTER_CONST_LONG( "STRETCH_CONDENSED", CondensedStretch );
	IMAGICK_REGISTER_CONST_LONG( "STRETCH_SEMICONDENSED", SemiCondensedStretch );
	IMAGICK_REGISTER_CONST_LONG( "STRETCH_SEMIEXPANDED", SemiExpandedStretch );
	IMAGICK_REGISTER_CONST_LONG( "STRETCH_EXPANDED", ExpandedStretch );
	IMAGICK_REGISTER_CONST_LONG( "STRETCH_EXTRAEXPANDED", ExtraExpandedStretch );
	IMAGICK_REGISTER_CONST_LONG( "STRETCH_ULTRAEXPANDED", UltraExpandedStretch );
	IMAGICK_REGISTER_CONST_LONG( "STRETCH_ANY", AnyStretch );
	IMAGICK_REGISTER_CONST_LONG( "ALIGN_UNDEFINED", UndefinedAlign );
	IMAGICK_REGISTER_CONST_LONG( "ALIGN_LEFT", LeftAlign );
	IMAGICK_REGISTER_CONST_LONG( "ALIGN_CENTER",	CenterAlign );
	IMAGICK_REGISTER_CONST_LONG( "ALIGN_RIGHT",	RightAlign );
	IMAGICK_REGISTER_CONST_LONG( "DECORATION_NO", NoDecoration );
	IMAGICK_REGISTER_CONST_LONG( "DECORATION_UNDERLINE", UnderlineDecoration );
	IMAGICK_REGISTER_CONST_LONG( "DECORATION_OVERLINE", OverlineDecoration );
	IMAGICK_REGISTER_CONST_LONG( "DECORATION_LINETROUGH", LineThroughDecoration );
	IMAGICK_REGISTER_CONST_LONG( "NOISE_UNIFORM", UniformNoise );
	IMAGICK_REGISTER_CONST_LONG( "NOISE_GAUSSIAN", GaussianNoise );
	IMAGICK_REGISTER_CONST_LONG( "NOISE_MULTIPLICATIVEGAUSSIAN", MultiplicativeGaussianNoise );
	IMAGICK_REGISTER_CONST_LONG( "NOISE_IMPULSE", ImpulseNoise );
	IMAGICK_REGISTER_CONST_LONG( "NOISE_LAPLACIAN", LaplacianNoise  );
	IMAGICK_REGISTER_CONST_LONG( "NOISE_POISSON", PoissonNoise );
	IMAGICK_REGISTER_CONST_LONG( "CHANNEL_UNDEFINED", UndefinedChannel );
	IMAGICK_REGISTER_CONST_LONG( "CHANNEL_RED", RedChannel );
	IMAGICK_REGISTER_CONST_LONG( "CHANNEL_CYAN", CyanChannel );
	IMAGICK_REGISTER_CONST_LONG( "CHANNEL_GREEN", GreenChannel );
	IMAGICK_REGISTER_CONST_LONG( "CHANNEL_MAGENTA", MagentaChannel );
	IMAGICK_REGISTER_CONST_LONG( "CHANNEL_BLUE", BlueChannel );
	IMAGICK_REGISTER_CONST_LONG( "CHANNEL_YELLOW", YellowChannel );
	IMAGICK_REGISTER_CONST_LONG( "CHANNEL_OPACITY", OpacityChannel );
	IMAGICK_REGISTER_CONST_LONG( "CHANNEL_BLACK", BlackChannel );
	IMAGICK_REGISTER_CONST_LONG( "CHANNEL_MATTE", MatteChannel );
	IMAGICK_REGISTER_CONST_LONG( "METRIC_UNDEFINED", UndefinedMetric );
	IMAGICK_REGISTER_CONST_LONG( "METRIC_MEANABSOLUTEERROR", MeanAbsoluteErrorMetric );
	IMAGICK_REGISTER_CONST_LONG( "METRIC_MEANSQUAREERROR", MeanSquaredErrorMetric );
	IMAGICK_REGISTER_CONST_LONG( "METRIC_PEAKABSOLUTEERROR", PeakAbsoluteErrorMetric );
	IMAGICK_REGISTER_CONST_LONG( "METRIC_PEAKSIGNALTONOISERATIO", PeakSignalToNoiseRatioMetric );
	IMAGICK_REGISTER_CONST_LONG( "METRIC_ROOTMEANSQUAREDERROR", RootMeanSquaredErrorMetric );
	IMAGICK_REGISTER_CONST_LONG( "PIXEL_CHAR", CharPixel );
	IMAGICK_REGISTER_CONST_LONG( "PIXEL_DOUBLE", DoublePixel );
	IMAGICK_REGISTER_CONST_LONG( "PIXEL_FLOAT", FloatPixel );
	IMAGICK_REGISTER_CONST_LONG( "PIXEL_INTEGER", IntegerPixel );
	IMAGICK_REGISTER_CONST_LONG( "PIXEL_LONG", LongPixel );
	IMAGICK_REGISTER_CONST_LONG( "PIXEL_QUANTUM", QuantumPixel );
	IMAGICK_REGISTER_CONST_LONG( "PIXEL_SHORT", ShortPixel );
	IMAGICK_REGISTER_CONST_LONG( "EVALUATE_UNDEFINED", UndefinedEvaluateOperator );
	IMAGICK_REGISTER_CONST_LONG( "EVALUATE_ADD", AddEvaluateOperator );
	IMAGICK_REGISTER_CONST_LONG( "EVALUATE_AND", AndEvaluateOperator );
	IMAGICK_REGISTER_CONST_LONG( "EVALUATE_DIVIDE", DivideEvaluateOperator );
	IMAGICK_REGISTER_CONST_LONG( "EVALUATE_LEFTSHIFT", LeftShiftEvaluateOperator );
	IMAGICK_REGISTER_CONST_LONG( "EVALUATE_MAX", MaxEvaluateOperator );
	IMAGICK_REGISTER_CONST_LONG( "EVALUATE_MIN", MinEvaluateOperator );
	IMAGICK_REGISTER_CONST_LONG( "EVALUATE_MULTIPLY", MultiplyEvaluateOperator );
	IMAGICK_REGISTER_CONST_LONG( "EVALUATE_OR", OrEvaluateOperator );
	IMAGICK_REGISTER_CONST_LONG( "EVALUATE_RIGHTSHIFT", RightShiftEvaluateOperator );
	IMAGICK_REGISTER_CONST_LONG( "EVALUATE_SET", SetEvaluateOperator );
	IMAGICK_REGISTER_CONST_LONG( "EVALUATE_SUBTRACT", SubtractEvaluateOperator );
	IMAGICK_REGISTER_CONST_LONG( "EVALUATE_XOR", XorEvaluateOperator );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_UNDEFINED", UndefinedColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_RGB", RGBColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_GRAY", GRAYColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_TRANSPARENT", TransparentColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_OHTA", OHTAColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_LAB", LABColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_XYZ", XYZColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_YCBCR", YCbCrColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_YCC", YCCColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_YIQ", YIQColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_YPBPR", YPbPrColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_YUV", YUVColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_CMYK", CMYKColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_SRGB", sRGBColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_HSB", HSBColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_HSL", HSLColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_HWB", HWBColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_REC601LUMA", Rec601LumaColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_REC709LUMA", Rec709LumaColorspace );
	IMAGICK_REGISTER_CONST_LONG( "COLORSPACE_LOG", LogColorspace );
	IMAGICK_REGISTER_CONST_LONG( "VIRTUALPIXELMETHOD_UNDEFINED", UndefinedVirtualPixelMethod );
	IMAGICK_REGISTER_CONST_LONG( "VIRTUALPIXELMETHOD_BACKGROUND", BackgroundVirtualPixelMethod );
	IMAGICK_REGISTER_CONST_LONG( "VIRTUALPIXELMETHOD_CONSTANT", ConstantVirtualPixelMethod );  /* deprecated */
	IMAGICK_REGISTER_CONST_LONG( "VIRTUALPIXELMETHOD_EDGE", EdgeVirtualPixelMethod );
	IMAGICK_REGISTER_CONST_LONG( "VIRTUALPIXELMETHOD_MIRROR", MirrorVirtualPixelMethod );
	IMAGICK_REGISTER_CONST_LONG( "VIRTUALPIXELMETHOD_TILE", TileVirtualPixelMethod );
	IMAGICK_REGISTER_CONST_LONG( "VIRTUALPIXELMETHOD_TRANSPARENT", TransparentVirtualPixelMethod );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_UNDEFINED", UndefinedPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_ROTATE", RotatePreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_SHEAR", ShearPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_ROLL", RollPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_HUE", HuePreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_SATURATION", SaturationPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_BRIGHTNESS", BrightnessPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_GAMMA", GammaPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_SPIFF", SpiffPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_DULL", DullPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_GRAYSCALE", GrayscalePreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_QUANTIZE", QuantizePreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_DESPECKLE", DespecklePreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_REDUCENOISE", ReduceNoisePreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_ADDNOISE", AddNoisePreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_SHARPEN", SharpenPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_BLUR", BlurPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_THRESHOLD", ThresholdPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_EDGEDETECT", EdgeDetectPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_SPREAD", SpreadPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_SOLARIZE", SolarizePreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_SHADE", ShadePreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_RAISE", RaisePreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_SEGMENT", SegmentPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_SWIRL", SwirlPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_IMPLODE", ImplodePreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_WAVE", WavePreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_OILPAINT", OilPaintPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_CHARCOALDRAWING", CharcoalDrawingPreview );
	IMAGICK_REGISTER_CONST_LONG( "PREVIEW_JPEG", JPEGPreview );
	IMAGICK_REGISTER_CONST_LONG( "RENDERINGINTENT_UNDEFINED", UndefinedIntent );
	IMAGICK_REGISTER_CONST_LONG( "RENDERINGINTENT_SATURATION", SaturationIntent );
	IMAGICK_REGISTER_CONST_LONG( "RENDERINGINTENT_PERCEPTUAL", PerceptualIntent );
	IMAGICK_REGISTER_CONST_LONG( "RENDERINGINTENT_ABSOLUTE", AbsoluteIntent );
	IMAGICK_REGISTER_CONST_LONG( "RENDERINGINTENT_RELATIVE", RelativeIntent );
	IMAGICK_REGISTER_CONST_LONG( "INTERLACE_UNDEFINED", UndefinedInterlace );
	IMAGICK_REGISTER_CONST_LONG( "INTERLACE_NO", NoInterlace );
	IMAGICK_REGISTER_CONST_LONG( "INTERLACE_LINE", LineInterlace );
	IMAGICK_REGISTER_CONST_LONG( "INTERLACE_PLANE", PlaneInterlace );
	IMAGICK_REGISTER_CONST_LONG( "INTERLACE_PARTITION", PartitionInterlace );
	IMAGICK_REGISTER_CONST_LONG( "FILLRULE_UNDEFINED", UndefinedRule );
	IMAGICK_REGISTER_CONST_LONG( "FILLRULE_EVENODD", EvenOddRule );
	IMAGICK_REGISTER_CONST_LONG( "FILLRULE_NONZERO", NonZeroRule );
	IMAGICK_REGISTER_CONST_LONG( "PATHUNITS_UNDEFINED", UndefinedPathUnits );
	IMAGICK_REGISTER_CONST_LONG( "PATHUNITS_USERSPACE", UserSpace );
	IMAGICK_REGISTER_CONST_LONG( "PATHUNITS_USERSPACEONUSE", UserSpaceOnUse );
	IMAGICK_REGISTER_CONST_LONG( "PATHUNITS_OBJECTBOUNDINGBOX", ObjectBoundingBox );
	IMAGICK_REGISTER_CONST_LONG( "LINECAP_UNDEFINED", UndefinedCap )
	IMAGICK_REGISTER_CONST_LONG( "LINECAP_BUTT", ButtCap );
	IMAGICK_REGISTER_CONST_LONG( "LINECAP_ROUND", RoundCap );
	IMAGICK_REGISTER_CONST_LONG( "LINECAP_SQUARE", SquareCap );
	IMAGICK_REGISTER_CONST_LONG( "LINEJOIN_UNDEFINED", UndefinedJoin );
	IMAGICK_REGISTER_CONST_LONG( "LINEJOIN_MITER", MiterJoin);
	IMAGICK_REGISTER_CONST_LONG( "LINEJOIN_ROUND", RoundJoin );
	IMAGICK_REGISTER_CONST_LONG( "LINEJOIN_BEVEL", BevelJoin );
	IMAGICK_REGISTER_CONST_LONG( "RESOURCETYPE_UNDEFINED", UndefinedResource );
	IMAGICK_REGISTER_CONST_LONG( "RESOURCETYPE_AREA", AreaResource );
	IMAGICK_REGISTER_CONST_LONG( "RESOURCETYPE_DISK", DiskResource );
	IMAGICK_REGISTER_CONST_LONG( "RESOURCETYPE_FILE", FileResource );
	IMAGICK_REGISTER_CONST_LONG( "RESOURCETYPE_MAP", MapResource );
	IMAGICK_REGISTER_CONST_LONG( "RESOURCETYPE_MEMORY", MemoryResource );

}




static void php_imagick_object_free_storage(void *object TSRMLS_DC)
{
	php_imagick_object *intern = (php_imagick_object *)object;


	if (!intern)
	{
		return;
	}

	if ( intern->magick_wand != (MagickWand *)NULL && IsMagickWand( intern->magick_wand ) )
	{
		ClearMagickWand( intern->magick_wand );
		intern->magick_wand = DestroyMagickWand( intern->magick_wand );
		intern->magick_wand = (MagickWand *)NULL;
	}

	zend_object_std_dtor(&intern->zo TSRMLS_CC);
	efree( intern );
}


static void php_imagickdraw_object_free_storage(void *object TSRMLS_DC)
{
	php_imagickdraw_object *intern = (php_imagickdraw_object *)object;

	if (!intern)
	{
		return;
	}

	if ( intern->drawing_wand != (DrawingWand *)NULL && IsDrawingWand( intern->drawing_wand ) )
	{
		ClearDrawingWand( intern->drawing_wand );
		intern->drawing_wand = DestroyDrawingWand( intern->drawing_wand );
		intern->drawing_wand = (DrawingWand *)NULL;
	}

	zend_object_std_dtor(&intern->zo TSRMLS_CC);
	efree( intern );
}


static void php_imagickpixeliterator_object_free_storage(void *object TSRMLS_DC)
{
	php_imagickpixeliterator_object *intern = (php_imagickpixeliterator_object *)object;

	if (!intern)
	{
		return;
	}

	if ( intern->pixel_iterator != (PixelIterator *)NULL && IsPixelIterator( intern->pixel_iterator ))
	{
		ClearPixelIterator( intern->pixel_iterator );
		intern->pixel_iterator = DestroyPixelIterator( intern->pixel_iterator );
		intern->pixel_iterator = (PixelIterator *)NULL;
	}

	zend_object_std_dtor(&intern->zo TSRMLS_CC);
	efree( intern );

}


static void php_imagickpixel_object_free_storage(void *object TSRMLS_DC)
{
	php_imagickpixel_object *intern = (php_imagickpixel_object *)object;

	if (!intern)
	{
		return;
	}

	if( intern->pixel_wand != (PixelWand *)NULL && IsPixelWand( intern->pixel_wand ) )
	{
		if ( intern->initialized_via_iterator < 1 )
		{
			ClearPixelWand( intern->pixel_wand );
			intern->pixel_wand = DestroyPixelWand( intern->pixel_wand );
			intern->pixel_wand = (PixelWand *)NULL;
		}
	}

	zend_object_std_dtor(&intern->zo TSRMLS_CC);
	efree( intern );
}


static zend_object_value php_imagick_object_new(zend_class_entry *class_type TSRMLS_DC)
{
	zval *tmp;
	zend_object_value retval;
	php_imagick_object *intern;

	/* Allocate memory for it */
	intern = emalloc( sizeof( php_imagick_object ) );
	memset( &intern->zo, 0, sizeof( php_imagick_object ) );

	/* Set the magickwand */
	intern->magick_wand = NewMagickWand();
	intern->numImages = 0;
	intern->next_out_of_bound = 0;

	/* ALLOC_HASHTABLE(intern->zo.properties); */

	zend_object_std_init(&intern->zo, class_type TSRMLS_CC);
	zend_hash_copy(intern->zo.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref,(void *) &tmp, sizeof(zval *));

	retval.handle = zend_objects_store_put(intern, NULL, (zend_objects_free_object_storage_t) php_imagick_object_free_storage, NULL TSRMLS_CC);
	retval.handlers = (zend_object_handlers *) &imagick_object_handlers;
	return retval;
}

static zend_object_value php_imagickdraw_object_new(zend_class_entry *class_type TSRMLS_DC)
{
	zval *tmp;
	zend_object_value retval;
	php_imagickdraw_object *intern;

	/* Allocate memory for it */
	intern = emalloc( sizeof( php_imagickdraw_object ) );
	memset( &intern->zo, 0, sizeof( php_imagickdraw_object ) );

	/* Set the magickwand */
	intern->drawing_wand = NewDrawingWand();

	/* ALLOC_HASHTABLE(intern->zo.properties); */

	zend_object_std_init(&intern->zo, class_type TSRMLS_CC);
	zend_hash_copy(intern->zo.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref,(void *) &tmp, sizeof(zval *));

	retval.handle = zend_objects_store_put(intern, NULL, (zend_objects_free_object_storage_t) php_imagickdraw_object_free_storage, NULL TSRMLS_CC);
	retval.handlers = (zend_object_handlers *) &imagickdraw_object_handlers;
	return retval;
}


static zend_object_value php_imagickpixeliterator_object_new(zend_class_entry *class_type TSRMLS_DC)
{
	zval *tmp;
	zend_object_value retval;
	php_imagickpixeliterator_object *intern;

	/* Allocate memory for it */
	intern = emalloc( sizeof( php_imagickpixeliterator_object ) );
	memset( &intern->zo, 0, sizeof( php_imagickpixeliterator_object ) );

	/* We cant initialize yet */
	intern->pixel_iterator = NULL;
	intern->instanciated_correctly = 0;

	/* ALLOC_HASHTABLE(intern->zo.properties); */

	zend_object_std_init(&intern->zo, class_type TSRMLS_CC);
	zend_hash_copy(intern->zo.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref,(void *) &tmp, sizeof(zval *));

	retval.handle = zend_objects_store_put(intern, NULL, (zend_objects_free_object_storage_t) php_imagickpixeliterator_object_free_storage, NULL TSRMLS_CC);
	retval.handlers = (zend_object_handlers *) &imagickpixeliterator_object_handlers;
	return retval;
}


static zend_object_value php_imagickpixel_object_new(zend_class_entry *class_type TSRMLS_DC)
{
	zval *tmp;
	zend_object_value retval;
	php_imagickpixel_object *intern;

	/* Allocate memory for it */
	intern = emalloc( sizeof( php_imagickpixel_object ) );
	memset( &intern->zo, 0, sizeof( php_imagickpixel_object ) );

	/* Set the magickwand */
	intern->pixel_wand = NULL;
	intern->initialized_via_iterator = 0;

	/* ALLOC_HASHTABLE(intern->zo.properties); */

	zend_object_std_init(&intern->zo, class_type TSRMLS_CC);
	zend_hash_copy(intern->zo.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref,(void *) &tmp, sizeof(zval *));

	retval.handle = zend_objects_store_put(intern, NULL, (zend_objects_free_object_storage_t) php_imagickpixel_object_free_storage, NULL TSRMLS_CC);
	retval.handlers = (zend_object_handlers *) &imagickpixel_object_handlers;
	return retval;
}


PHP_MINIT_FUNCTION(imagick)
{
	zend_class_entry ce;

	/*
		Allocate some memory
	*/
	memcpy( &imagick_object_handlers, zend_get_std_object_handlers(), sizeof( zend_object_handlers ) );
	memcpy( &imagickdraw_object_handlers, zend_get_std_object_handlers(), sizeof( zend_object_handlers ) );
	memcpy( &imagickpixeliterator_object_handlers, zend_get_std_object_handlers(), sizeof( zend_object_handlers ) );
	memcpy( &imagickpixel_object_handlers, zend_get_std_object_handlers(), sizeof( zend_object_handlers ) );

	/* Setup magickwand env */
	MagickWandGenesis();

	/*
		Initialize exceptions (Imagick exception)
	*/
	INIT_CLASS_ENTRY(ce, PHP_IMAGICK_EXCEPTION_SC_NAME, NULL);
	php_imagick_exception_class_entry = zend_register_internal_class_ex(&ce, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
	php_imagick_exception_class_entry->ce_flags |= ZEND_ACC_FINAL;

	/*
	Initialize exceptions (ImagickDraw exception)
	*/
	INIT_CLASS_ENTRY(ce, PHP_IMAGICKDRAW_EXCEPTION_SC_NAME, NULL);
	php_imagickdraw_exception_class_entry = zend_register_internal_class_ex(&ce, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
	php_imagickdraw_exception_class_entry->ce_flags |= ZEND_ACC_FINAL;

	/*
	Initialize exceptions (ImagickPixelIterator exception)
	*/
	INIT_CLASS_ENTRY(ce, PHP_IMAGICKPIXELITERATOR_EXCEPTION_SC_NAME, NULL);
	php_imagickpixeliterator_sc_entry = zend_register_internal_class_ex(&ce, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
	php_imagickpixeliterator_sc_entry->ce_flags |= ZEND_ACC_FINAL;

	/*
	Initialize exceptions (ImagickPixel exception)
	*/
	INIT_CLASS_ENTRY(ce, PHP_IMAGICKPIXEL_EXCEPTION_SC_NAME, NULL);
	php_imagickpixel_exception_class_entry = zend_register_internal_class_ex(&ce, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
	php_imagickpixel_exception_class_entry->ce_flags |= ZEND_ACC_FINAL;

	/*
		Initialize the class (Imagick)
	*/
	INIT_CLASS_ENTRY(ce, PHP_IMAGICK_SC_NAME, php_imagick_class_methods);
	ce.create_object = php_imagick_object_new;
	imagick_object_handlers.clone_obj = NULL;
	php_imagick_sc_entry = zend_register_internal_class(&ce TSRMLS_CC);
	zend_class_implements(php_imagick_sc_entry TSRMLS_CC, 1, zend_ce_iterator);

	/*
		Initialize the class (ImagickDraw)
	*/
	INIT_CLASS_ENTRY(ce, PHP_IMAGICKDRAW_SC_NAME, php_imagickdraw_class_methods);
	ce.create_object = php_imagickdraw_object_new;
	imagickdraw_object_handlers.clone_obj = NULL;
	php_imagickdraw_sc_entry = zend_register_internal_class(&ce TSRMLS_CC);

	/*
		Initialize the class (ImagickPixelIterator)
	*/
	INIT_CLASS_ENTRY(ce, PHP_IMAGICKPIXELITERATOR_SC_NAME, php_imagickpixeliterator_class_methods);
	ce.create_object = php_imagickpixeliterator_object_new;
	imagickpixeliterator_object_handlers.clone_obj = NULL;
	php_imagickpixeliterator_sc_entry = zend_register_internal_class(&ce TSRMLS_CC);

	/*
		Initialize the class (ImagickPixel)
	*/
	INIT_CLASS_ENTRY(ce, PHP_IMAGICKPIXEL_SC_NAME, php_imagickpixel_class_methods);
	ce.create_object = php_imagickpixel_object_new;
	imagickpixel_object_handlers.clone_obj = NULL;
	php_imagickpixel_sc_entry = zend_register_internal_class(&ce TSRMLS_CC);

	initializeMagickConstants();

	return SUCCESS;
}


PHP_MINFO_FUNCTION(imagick)
{
	const char *imageMagickReleaseDate = MagickGetReleaseDate();
	unsigned long versionNumber;
	const char *imageMagickVersion = MagickGetVersion( &versionNumber );

	char **supportedFormats;
	unsigned long numFormats = 0;
	char *pattern = "*";
	int i;
	char formats[2056];
	char buffer[4];

	supportedFormats = (char **) MagickQueryFormats( pattern, &numFormats );
	sprintf( buffer, "%ld", numFormats );

	php_info_print_table_start();
	php_info_print_table_row( 2, "imagick module", "enabled" );
	php_info_print_table_row( 2, "imagick module version", PHP_IMAGICK_EXTVER );
	php_info_print_table_row( 2, "imagick classes", "Imagick, ImagickDraw, ImagickPixel, ImagickPixelIterator" );
	php_info_print_table_row( 2, "ImageMagick version", imageMagickVersion );
	php_info_print_table_row( 2, "ImageMagick release date", imageMagickReleaseDate );
	php_info_print_table_row( 2, "ImageMagick Number of supported formats: ", buffer );

	memset( formats, '\0', sizeof( formats ) );

	if ( ( numFormats > 0 ) && supportedFormats != (char **)NULL )
	{
		for( i = 0 ; i < numFormats ; i++ )
		{
			strcat( formats, supportedFormats[i] );
			if ( i != ( numFormats - 1 ) )
			{
				strcat( formats, ", " );
			}
			IMAGICK_FREE_MEMORY( char *, supportedFormats[i] );
		}
		php_info_print_table_row( 2, "ImageMagick Supported formats", formats );
	}

	php_info_print_table_end();
	IMAGICK_FREE_MEMORY( char **, supportedFormats );
}


PHP_MSHUTDOWN_FUNCTION(imagick)
{
	/* Destroy the magick wand env */
	MagickWandTerminus();
	return( SUCCESS );
}


zend_module_entry imagick_module_entry =
{
	#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
	#endif
	PHP_IMAGICK_EXTNAME,
	php_imagick_functions,			/* Functions */
	PHP_MINIT(imagick),			/* MINIT */
	PHP_MSHUTDOWN(imagick),		/* MSHUTDOWN */
	NULL,						    /* RINIT */
	NULL,						    /* RSHUTDOWN */
	PHP_MINFO(imagick),			/* MINFO */
	#if ZEND_MODULE_API_NO >= 20010901
	PHP_IMAGICK_EXTVER,
	#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_IMAGICK
ZEND_GET_MODULE(imagick)
#endif
