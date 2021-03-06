/* DO NOT EDIT THIS FILE - it is machine generated */
#ifdef __APPLE__
	#include <JavaVM/jni.h>
#else
	#include <jni.h>
#endif

/* Header for class arc_api_image_ArcImageJAPI */

#ifndef _Included_arc_api_image_ArcImageJAPI
#define _Included_arc_api_image_ArcImageJAPI
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    Histogram
 * Signature: (JII)[Ljava/awt/geom/Point2D/Float;
 */
JNIEXPORT jobjectArray JNICALL Java_arc_api_image_ArcImageJAPI_Histogram__JII
  (JNIEnv *, jclass, jlong, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    Histogram
 * Signature: (JIIIIII)[Ljava/awt/geom/Point2D/Float;
 */
JNIEXPORT jobjectArray JNICALL Java_arc_api_image_ArcImageJAPI_Histogram__JIIIIII
  (JNIEnv *, jclass, jlong, jint, jint, jint, jint, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    GetRow
 * Signature: (JIIIII)[Ljava/awt/geom/Point2D/Float;
 */
JNIEXPORT jobjectArray JNICALL Java_arc_api_image_ArcImageJAPI_GetRow
  (JNIEnv *, jclass, jlong, jint, jint, jint, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    GetCol
 * Signature: (JIIIII)[Ljava/awt/geom/Point2D/Float;
 */
JNIEXPORT jobjectArray JNICALL Java_arc_api_image_ArcImageJAPI_GetCol
  (JNIEnv *, jclass, jlong, jint, jint, jint, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    GetRowArea
 * Signature: (JIIIIII)[Ljava/awt/geom/Point2D/Float;
 */
JNIEXPORT jobjectArray JNICALL Java_arc_api_image_ArcImageJAPI_GetRowArea
  (JNIEnv *, jclass, jlong, jint, jint, jint, jint, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    GetColArea
 * Signature: (JIIIIII)[Ljava/awt/geom/Point2D/Float;
 */
JNIEXPORT jobjectArray JNICALL Java_arc_api_image_ArcImageJAPI_GetColArea
  (JNIEnv *, jclass, jlong, jint, jint, jint, jint, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    GetStats
 * Signature: (JII)Larc/api/image/ImageStats;
 */
JNIEXPORT jobject JNICALL Java_arc_api_image_ArcImageJAPI_GetStats__JII
  (JNIEnv *, jclass, jlong, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    GetStats
 * Signature: (JIIIIII)Larc/api/image/ImageStats;
 */
JNIEXPORT jobject JNICALL Java_arc_api_image_ArcImageJAPI_GetStats__JIIIIII
  (JNIEnv *, jclass, jlong, jint, jint, jint, jint, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    GetDifStats
 * Signature: (JJII)Larc/api/image/ImageDifStats;
 */
JNIEXPORT jobject JNICALL Java_arc_api_image_ArcImageJAPI_GetDifStats__JJII
  (JNIEnv *, jclass, jlong, jlong, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    GetDifStats
 * Signature: (JJIIIIII)Larc/api/image/ImageDifStats;
 */
JNIEXPORT jobject JNICALL Java_arc_api_image_ArcImageJAPI_GetDifStats__JJIIIIII
  (JNIEnv *, jclass, jlong, jlong, jint, jint, jint, jint, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    Add
 * Signature: (JJJII)V
 */
JNIEXPORT void JNICALL Java_arc_api_image_ArcImageJAPI_Add
  (JNIEnv *, jclass, jlong, jlong, jlong, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    Subtract
 * Signature: (JJII)V
 */
JNIEXPORT void JNICALL Java_arc_api_image_ArcImageJAPI_Subtract
  (JNIEnv *, jclass, jlong, jlong, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    SubtractHalves
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_arc_api_image_ArcImageJAPI_SubtractHalves
  (JNIEnv *, jclass, jlong, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    Divide
 * Signature: (JJII)V
 */
JNIEXPORT void JNICALL Java_arc_api_image_ArcImageJAPI_Divide
  (JNIEnv *, jclass, jlong, jlong, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    Copy
 * Signature: (JIIJII)V
 */
JNIEXPORT void JNICALL Java_arc_api_image_ArcImageJAPI_Copy__JIIJII
  (JNIEnv *, jclass, jlong, jint, jint, jlong, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    Copy
 * Signature: (JIJI)V
 */
JNIEXPORT void JNICALL Java_arc_api_image_ArcImageJAPI_Copy__JIJI
  (JNIEnv *, jclass, jlong, jint, jlong, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    Fill
 * Signature: (JIII)V
 */
JNIEXPORT void JNICALL Java_arc_api_image_ArcImageJAPI_Fill
  (JNIEnv *, jclass, jlong, jint, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    GradientFill
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_arc_api_image_ArcImageJAPI_GradientFill
  (JNIEnv *, jclass, jlong, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    SmileyFill
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_arc_api_image_ArcImageJAPI_SmileyFill
  (JNIEnv *, jclass, jlong, jint, jint);

/*
 * Class:     arc_api_image_ArcImageJAPI
 * Method:    VerifyAsSynthetic
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_arc_api_image_ArcImageJAPI_VerifyAsSynthetic
  (JNIEnv *, jclass, jlong, jint, jint);

#ifdef __cplusplus
}
#endif
#endif
