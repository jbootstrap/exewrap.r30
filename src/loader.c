#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <process.h>
#include <shlobj.h>
#include <stdio.h>
#include <jni.h>

#include "include/jvm.h"
#include "include/loader.h"
#include "include/eventlog.h"
#include "include/message.h"

BOOL            LoadMainClass(int argc, char* argv[], char* utilities, LOAD_RESULT* result);
BOOL            SetSplashScreenResource(char* splash_screen_name, BYTE* splash_screen_image_buf, DWORD splash_screen_image_len);
DWORD   WINAPI  RemoteCallMainMethod(void* _shared_memory_handle);
char*           ToString(JNIEnv* env, jobject object, char* buf);
char*           GetModuleObjectName(const char* prefix);
BYTE*           GetResource(LPCTSTR name, RESOURCE* resource);
char*           GetWinErrorMessage(DWORD err, int* exit_code, char* buf);
char*           GetJniErrorMessage(int err, int* exit_code, char* buf);
void    JNICALL JNI_WriteConsole(JNIEnv *env, jobject clazz, jbyteArray b, jint off, jint len);
void    JNICALL JNI_WriteEventLog(JNIEnv *env, jobject clazz, jint logType, jstring message);
void    JNICALL JNI_UncaughtException(JNIEnv *env, jobject clazz, jstring thread, jstring message, jstring trace);
jstring JNICALL JNI_SetEnvironment(JNIEnv *env, jobject clazz, jstring key, jstring value);

static jint   register_native(JNIEnv* env, jclass cls, const char* name, const char* signature, void* fnPtr);
static void   print_stack_trace(const char* text);
static char** split_args(char* buffer, int* p_argc);

extern void   OutputConsole(BYTE* buf, DWORD len);
extern void   OutputMessage(const char* text);
extern UINT   UncaughtException(const char* thread, const char* message, const char* trace);

static jclass    ExewrapClassLoader = NULL;
static jobject   exewrapClassLoader = NULL;
static jclass    MainClass = NULL;
static jmethodID MainClass_main = NULL;

BOOL LoadMainClass(int argc, char* argv[], char* utilities, LOAD_RESULT* result)
{
	RESOURCE     res;
	jclass       ClassLoader;
	jmethodID    ClassLoader_getSystemClassLoader;
	jmethodID    ClassLoader_definePackage;
	jobject      systemClassLoader;
	jclass       JarInputStream;
	jmethodID    JarInputStream_init;
	jclass       ByteBufferInputStream;
	jmethodID    ByteBufferInputStream_init;
	jobjectArray jars;
	jclass       URLConnection;
	jclass       URLStreamHandler;
	jmethodID    ExewrapClassLoader_init;
	jmethodID    exewrapClassLoader_register;
	jmethodID    exewrapClassLoader_loadUtilities;
	jmethodID    exewrapClassLoader_getMainClass;

	// ClassLoader
	ClassLoader = (*env)->FindClass(env, "java/lang/ClassLoader");
	if (ClassLoader == NULL)
	{
		result->msg_id = MSG_ID_ERR_DEFINE_CLASS;
		sprintf(result->msg, _(MSG_ID_ERR_DEFINE_CLASS), "java.lang.ClassLoader");
		goto EXIT;
	}
	ClassLoader_definePackage = (*env)->GetMethodID(env, ClassLoader, "definePackage", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/net/URL;)Ljava/lang/Package;");
	if (ClassLoader_definePackage == NULL)
	{
		result->msg_id = MSG_ID_ERR_GET_METHOD;
		sprintf(result->msg, _(MSG_ID_ERR_GET_METHOD), "java.lang.ClassLoader.definePackage(java.lang.String, java.lang.String, java.lang.String, java.lang.String, java.lang.String, java.lang.String, java.lang.String, java.net.URL)");
		goto EXIT;
	}
	ClassLoader_getSystemClassLoader = (*env)->GetStaticMethodID(env, ClassLoader, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
	if (ClassLoader_getSystemClassLoader == NULL)
	{
		result->msg_id = MSG_ID_ERR_GET_METHOD;
		sprintf(result->msg, _(MSG_ID_ERR_GET_METHOD), "java.lang.ClassLoader.getSystemClassLoader()");
		goto EXIT;
	}
	systemClassLoader = (*env)->CallStaticObjectMethod(env, ClassLoader, ClassLoader_getSystemClassLoader);
	if (systemClassLoader == NULL)
	{
		//ignore
	}
	else
	{
		// Define package "exewrap.core"
		jstring packageName = GetJString(env, "exewrap.core");
		(*env)->CallObjectMethod(env, systemClassLoader, ClassLoader_definePackage, packageName, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	}

	// JarInputStream
	JarInputStream = (*env)->FindClass(env, "java/util/jar/JarInputStream");
	if (JarInputStream == NULL)
	{
		result->msg_id = MSG_ID_ERR_DEFINE_CLASS;
		sprintf(result->msg, _(MSG_ID_ERR_DEFINE_CLASS), "java.util.jar.JarInputStream");
		goto EXIT;
	}
	JarInputStream_init = (*env)->GetMethodID(env, JarInputStream, "<init>", "(Ljava/io/InputStream;)V");
	if (JarInputStream_init == NULL)
	{
		result->msg_id = MSG_ID_ERR_GET_CONSTRUCTOR;
		sprintf(result->msg, _(MSG_ID_ERR_GET_CONSTRUCTOR), "java.util.jar.JarInputStream(java.io.InputStream)");
		goto EXIT;
	}

	// ByteBufferInputStream
	if (GetResource("BYTE_BUFFER_INPUT_STREAM", &res) == NULL)
	{
		result->msg_id = MSG_ID_ERR_RESOURCE_NOT_FOUND;
		sprintf(result->msg, _(MSG_ID_ERR_RESOURCE_NOT_FOUND), "RT_RCDATA: BYTE_BUFFER_INPUT_STREAM");
		goto EXIT;
	}
	ByteBufferInputStream = (*env)->DefineClass(env, "exewrap/core/ByteBufferInputStream", systemClassLoader, res.buf, res.len);
	if (ByteBufferInputStream == NULL)
	{
		result->msg_id = MSG_ID_ERR_DEFINE_CLASS;
		sprintf(result->msg, _(MSG_ID_ERR_DEFINE_CLASS), "exewrap.core.ByteBufferInputStream");
		goto EXIT;
	}
	ByteBufferInputStream_init = (*env)->GetMethodID(env, ByteBufferInputStream, "<init>", "(Ljava/nio/ByteBuffer;)V");
	if (ByteBufferInputStream_init == NULL)
	{
		result->msg_id = MSG_ID_ERR_GET_CONSTRUCTOR;
		sprintf(result->msg, _(MSG_ID_ERR_GET_CONSTRUCTOR), "exewrap.core.ByteBufferInputStream(java.nio.ByteBuffer)");
		goto EXIT;
	}

	// JarInputStream[] jars = new JarInputStream[2];
	jars = (*env)->NewObjectArray(env, 2, JarInputStream, NULL);
	if (jars == NULL)
	{
		result->msg_id = MSG_ID_ERR_NEW_OBJECT;
		sprintf(result->msg, _(MSG_ID_ERR_NEW_OBJECT), "JarInputStream[]");
		goto EXIT;
	}

	//util.jar
	{
		jobject byteBuffer;
		jobject byteBufferInputStream = NULL;
		jobject jarInputStream = NULL;

		if (GetResource("UTIL_JAR", &res) == NULL)
		{
			result->msg_id = MSG_ID_ERR_RESOURCE_NOT_FOUND;
			sprintf(result->msg, _(MSG_ID_ERR_RESOURCE_NOT_FOUND), "RT_RCDATA: UTIL_JAR");
			goto EXIT;
		}
		byteBuffer = (*env)->NewDirectByteBuffer(env, res.buf, res.len);
		if (byteBuffer == NULL)
		{
			result->msg_id = MSG_ID_ERR_NEW_OBJECT;
			sprintf(result->msg, _(MSG_ID_ERR_NEW_OBJECT), "NewDirectByteBuffer(JNIEnv* env, void* address, jlong capacity)");
			goto EXIT;
		}
		byteBufferInputStream = (*env)->NewObject(env, ByteBufferInputStream, ByteBufferInputStream_init, byteBuffer);
		if (byteBufferInputStream == NULL)
		{
			result->msg_id = MSG_ID_ERR_NEW_OBJECT;
			sprintf(result->msg, _(MSG_ID_ERR_NEW_OBJECT), "exewrap.core.ByteBufferInputStream(java.nio.ByteBuffer)");
			goto EXIT;
		}
		jarInputStream = (*env)->NewObject(env, JarInputStream, JarInputStream_init, byteBufferInputStream);
		if (jarInputStream == NULL)
		{
			result->msg_id = MSG_ID_ERR_NEW_OBJECT;
			sprintf(result->msg, _(MSG_ID_ERR_NEW_OBJECT), "java.util.jar.JarInputStream(exewrap.core.ByteBufferInputStream)");
			goto EXIT;
		}
		(*env)->SetObjectArrayElement(env, jars, 0, jarInputStream);
	}

	// user.jar or user.pack.gz
	{
		BOOL     isPackGz = TRUE;
		jobject  byteBuffer;
		jobject  byteBufferInputStream;
		jobject  jarInputStream;

		if (GetResource("PACK_GZ", &res) == NULL)
		{
			isPackGz = FALSE;
			if (GetResource("JAR", &res) == NULL)
			{
				result->msg_id = MSG_ID_ERR_RESOURCE_NOT_FOUND;
				sprintf(result->msg, _(MSG_ID_ERR_RESOURCE_NOT_FOUND), "RT_RCDATA: JAR, RT_RCDATA: PACK_GZ");
				goto EXIT;
			}
		}
		byteBuffer = (*env)->NewDirectByteBuffer(env, res.buf, res.len);
		if (byteBuffer == NULL)
		{
			result->msg_id = MSG_ID_ERR_NEW_OBJECT;
			sprintf(result->msg, _(MSG_ID_ERR_NEW_OBJECT), "NewDirectByteBuffer(JNIEnv* env, void* address, jlong capacity)");
			goto EXIT;
		}
		byteBufferInputStream = (*env)->NewObject(env, ByteBufferInputStream, ByteBufferInputStream_init, byteBuffer);
		if (byteBufferInputStream == NULL)
		{
			result->msg_id = MSG_ID_ERR_NEW_OBJECT;
			sprintf(result->msg, _(MSG_ID_ERR_NEW_OBJECT), "exewrap.core.ByteBufferInputStream(java.nio.ByteBuffer)");
			goto EXIT;
		}
		if (isPackGz)
		{
			jclass    GZIPInputStream;
			jmethodID GZIPInputStream_init;
			jobject   gzipInputStream;
			jclass    PackInputStream;
			jmethodID PackInputStream_init;
			jobject   packInputStream;

			// GZIPInputStream
			GZIPInputStream = (*env)->FindClass(env, "java/util/zip/GZIPInputStream");
			if (GZIPInputStream == NULL)
			{
				result->msg_id = MSG_ID_ERR_DEFINE_CLASS;
				sprintf(result->msg, _(MSG_ID_ERR_DEFINE_CLASS), "java.util.zip.GZIPInputStream");
				goto EXIT;
			}
			GZIPInputStream_init = (*env)->GetMethodID(env, GZIPInputStream, "<init>", "(Ljava/io/InputStream;)V");
			if (GZIPInputStream_init == NULL)
			{
				result->msg_id = MSG_ID_ERR_GET_CONSTRUCTOR;
				sprintf(result->msg, _(MSG_ID_ERR_GET_CONSTRUCTOR), "java.util.zip.GZIPInputStream(java.io.InputStream)");
				goto EXIT;
			}
			gzipInputStream = (*env)->NewObject(env, GZIPInputStream, GZIPInputStream_init, byteBufferInputStream);
			if (gzipInputStream == NULL)
			{
				result->msg_id = MSG_ID_ERR_NEW_OBJECT;
				sprintf(result->msg, _(MSG_ID_ERR_NEW_OBJECT), "java.util.zip.GZIPInputStream(ByteBufferInputStream)");
				goto EXIT;
			}

			// PackInputStream
			if (GetResource("PACK_INPUT_STREAM", &res) == NULL)
			{
				result->msg_id = MSG_ID_ERR_RESOURCE_NOT_FOUND;
				sprintf(result->msg, _(MSG_ID_ERR_RESOURCE_NOT_FOUND), "RT_RCDATA: PACK_INPUT_STREAM");
				goto EXIT;
			}
			PackInputStream = (*env)->DefineClass(env, "exewrap/core/PackInputStream", systemClassLoader, res.buf, res.len);
			if (PackInputStream == NULL)
			{
				result->msg_id = MSG_ID_ERR_DEFINE_CLASS;
				sprintf(result->msg, _(MSG_ID_ERR_DEFINE_CLASS), "exewrap.core.PackInputStream");
				goto EXIT;
			}
			PackInputStream_init = (*env)->GetMethodID(env, PackInputStream, "<init>", "(Ljava/io/InputStream;)V");
			if (PackInputStream_init == NULL)
			{
				result->msg_id = MSG_ID_ERR_GET_CONSTRUCTOR;
				sprintf(result->msg, _(MSG_ID_ERR_GET_CONSTRUCTOR), "exewrap.core.PackInputStream(java.io.InputStream)");
				goto EXIT;
			}
			packInputStream = (*env)->NewObject(env, PackInputStream, PackInputStream_init, gzipInputStream);
			if (packInputStream == NULL)
			{
				result->msg_id = MSG_ID_ERR_NEW_OBJECT;
				sprintf(result->msg, _(MSG_ID_ERR_NEW_OBJECT), "exewrap.core.PackInputStream(java.util.zip.GZIPInputStream)");
				goto EXIT;
			}

			// JarInputStream
			jarInputStream = (*env)->NewObject(env, JarInputStream, JarInputStream_init, packInputStream);
			if (jarInputStream == NULL)
			{
				result->msg_id = MSG_ID_ERR_NEW_OBJECT;
				sprintf(result->msg, _(MSG_ID_ERR_NEW_OBJECT), "java.util.jar.JarInputStream(PackInputStream)");
				goto EXIT;
			}
		}
		else
		{
			// JarInputStream
			jarInputStream = (*env)->NewObject(env, JarInputStream, JarInputStream_init, byteBufferInputStream);
			if (jarInputStream == NULL)
			{
				result->msg_id = MSG_ID_ERR_NEW_OBJECT;
				sprintf(result->msg, _(MSG_ID_ERR_NEW_OBJECT), "java.util.jar.JarInputStream(ByteBufferInputStream)");
				goto EXIT;
			}
		}
		(*env)->SetObjectArrayElement(env, jars, 1, jarInputStream);
	}

	// URLConnection
	if (GetResource("URL_CONNECTION", &res) == NULL)
	{
		result->msg_id = MSG_ID_ERR_RESOURCE_NOT_FOUND;
		sprintf(result->msg, _(MSG_ID_ERR_RESOURCE_NOT_FOUND), "RT_RCDATA: URL_CONNECTION");
		goto EXIT;
	}
	URLConnection = (*env)->DefineClass(env, "exewrap/core/URLConnection", systemClassLoader, res.buf, res.len);
	if (URLConnection == NULL)
	{
		result->msg_id = MSG_ID_ERR_DEFINE_CLASS;
		sprintf(result->msg, _(MSG_ID_ERR_DEFINE_CLASS), "exewrap.core.URLConnection");
		goto EXIT;
	}
	// URLStreamHandler
	if (GetResource("URL_STREAM_HANDLER", &res) == NULL)
	{
		result->msg_id = MSG_ID_ERR_RESOURCE_NOT_FOUND;
		sprintf(result->msg, _(MSG_ID_ERR_RESOURCE_NOT_FOUND), "RT_RCDATA: URL_STREAM_HANDLER");
		goto EXIT;
	}
	URLStreamHandler = (*env)->DefineClass(env, "exewrap/core/URLStreamHandler", systemClassLoader, res.buf, res.len);
	if (URLStreamHandler == NULL)
	{
		result->msg_id = MSG_ID_ERR_DEFINE_CLASS;
		sprintf(result->msg, _(MSG_ID_ERR_DEFINE_CLASS), "exewrap.core.URLStreamHandler");
		goto EXIT;
	}
	// ExewrapClassLoader
	if (GetResource("EXEWRAP_CLASS_LOADER", &res) == NULL)
	{
		result->msg_id = MSG_ID_ERR_RESOURCE_NOT_FOUND;
		sprintf(result->msg, _(MSG_ID_ERR_RESOURCE_NOT_FOUND), "RT_RCDATA: EXEWRAP_CLASS_LOADER");
		goto EXIT;
	}
	ExewrapClassLoader = (*env)->DefineClass(env, "exewrap/core/ExewrapClassLoader", systemClassLoader, res.buf, res.len);
	if (ExewrapClassLoader == NULL)
	{
		result->msg_id = MSG_ID_ERR_DEFINE_CLASS;
		sprintf(result->msg, _(MSG_ID_ERR_DEFINE_CLASS), "exewrap.core.ExewrapClassLoader");
		goto EXIT;
	}
	// register native methods
	if (register_native(env, ExewrapClassLoader, "WriteConsole", "([BII)V", JNI_WriteConsole) != 0)
	{
		result->msg_id = MSG_ID_ERR_REGISTER_NATIVE;
		sprintf(result->msg, _(MSG_ID_ERR_REGISTER_NATIVE), "WriteConsole");
		goto EXIT;
	}
	if (register_native(env, ExewrapClassLoader, "WriteEventLog", "(ILjava/lang/String;)V", JNI_WriteEventLog) != 0)
	{
		result->msg_id = MSG_ID_ERR_REGISTER_NATIVE;
		sprintf(result->msg, _(MSG_ID_ERR_REGISTER_NATIVE), "WriteEventLog");
		goto EXIT;
	}
	if (register_native(env, ExewrapClassLoader, "UncaughtException", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", JNI_UncaughtException) != 0)
	{
		result->msg_id = MSG_ID_ERR_REGISTER_NATIVE;
		sprintf(result->msg, _(MSG_ID_ERR_REGISTER_NATIVE), "UncaughtException");
		goto EXIT;
	}
	if (register_native(env, ExewrapClassLoader, "SetEnvironment", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", JNI_SetEnvironment) != 0)
	{
		result->msg_id = MSG_ID_ERR_REGISTER_NATIVE;
		sprintf(result->msg, _(MSG_ID_ERR_REGISTER_NATIVE), "SetEnvironment");
		goto EXIT;
	}
	// Create ExewrapClassLoader instance
	ExewrapClassLoader_init = (*env)->GetMethodID(env, ExewrapClassLoader, "<init>", "(Ljava/lang/ClassLoader;[Ljava/util/jar/JarInputStream;)V");
	if (ExewrapClassLoader_init == NULL)
	{
		result->msg_id = MSG_ID_ERR_GET_CONSTRUCTOR;
		sprintf(result->msg, _(MSG_ID_ERR_GET_CONSTRUCTOR), "exewrap.core.ExewrapClassLoader(java.lang.ClassLoader, java.util.jar.JarInputStream[])");
		goto EXIT;
	}
	// ExewrapClassLoader exewrarpClassLoader = new ExewrapClassLoader(ClassLoader.getSystemClasssLoader());
	exewrapClassLoader = (*env)->NewObject(env, ExewrapClassLoader, ExewrapClassLoader_init, systemClassLoader, jars);
	if (exewrapClassLoader == NULL)
	{
		result->msg_id = MSG_ID_ERR_NEW_OBJECT;
		sprintf(result->msg, _(MSG_ID_ERR_NEW_OBJECT), "exewrap.core.ExewrapClassLoader(java.lang.ClassLoader, java.util.jar.JarInputStream[])");
		goto EXIT;
	}
	exewrapClassLoader_register = (*env)->GetMethodID(env, ExewrapClassLoader, "register", "()V");
	if (exewrapClassLoader_register == NULL)
	{
		result->msg_id = MSG_ID_ERR_GET_METHOD;
		sprintf(result->msg, _(MSG_ID_ERR_GET_METHOD), "exewrap.core.ExewrapClassLoader.register()");
		goto EXIT;
	}
	exewrapClassLoader_loadUtilities = (*env)->GetMethodID(env, ExewrapClassLoader, "loadUtilities", "(Ljava/lang/String;)V");
	if (exewrapClassLoader_loadUtilities == NULL)
	{
		result->msg_id = MSG_ID_ERR_GET_METHOD;
		sprintf(result->msg, _(MSG_ID_ERR_GET_METHOD), "exewrap.core.ExewrapClassLoader.loadUtilities(java.lang.String)");
		goto EXIT;
	}

	// exewrapClassLoader.register();
	(*env)->CallObjectMethod(env, exewrapClassLoader, exewrapClassLoader_register);

	// exewrapClassLoader.loadUtilities();
	{
		jstring s = GetJString(env, utilities);
		(*env)->CallObjectMethod(env, exewrapClassLoader, exewrapClassLoader_loadUtilities, s);
	}

	// MainClass
	exewrapClassLoader_getMainClass = (*env)->GetMethodID(env, ExewrapClassLoader, "getMainClass", "(Ljava/lang/String;)Ljava/lang/Class;");
	if (exewrapClassLoader_getMainClass == NULL)
	{
		result->msg_id = MSG_ID_ERR_GET_METHOD;
		sprintf(result->msg, _(MSG_ID_ERR_GET_METHOD), "exewrap.core.ExewrapClassLoader.getMainClass(java.lang.String)");
		goto EXIT;
	}
	MainClass = (*env)->CallObjectMethod(env, exewrapClassLoader, exewrapClassLoader_getMainClass, GetJString(env, GetResource("MAIN_CLASS", NULL)));
	if (MainClass == NULL)
	{
		result->msg_id = MSG_ID_ERR_LOAD_MAIN_CLASS;
		strcpy(result->msg, _(MSG_ID_ERR_LOAD_MAIN_CLASS));
		goto EXIT;
	}
	MainClass_main = (*env)->GetStaticMethodID(env, MainClass, "main", "([Ljava/lang/String;)V");
	if (MainClass_main == NULL)
	{
		result->msg_id = MSG_ID_ERR_FIND_MAIN_METHOD;
		strcpy(result->msg, _(MSG_ID_ERR_FIND_MAIN_METHOD));
		goto EXIT;
	}
	result->MainClass_main_args = (*env)->NewObjectArray(env, argc - 1, (*env)->FindClass(env, "java/lang/String"), NULL);
	{
		int i;
		for (i = 1; i < argc; i++)
		{
			(*env)->SetObjectArrayElement(env, result->MainClass_main_args, (i - 1), GetJString(env, argv[i]));
		}
	}
	result->MainClass = MainClass;
	result->MainClass_main = MainClass_main;
	result->msg_id = 0;
	*(result->msg) = '\0';
	return TRUE;

EXIT:
	if ((*env)->ExceptionCheck(env) == JNI_TRUE)
	{
		(*env)->ExceptionDescribe(env);
		(*env)->ExceptionClear(env);
	}

	return FALSE;
}


BOOL SetSplashScreenResource(char* splash_screen_name, BYTE* splash_screen_image_buf, DWORD splash_screen_image_len)
{
	BOOL       ret = FALSE;
	jstring    name;
	jbyteArray image;
	jmethodID  exewrapClassLoader_setSplashScreenResource;

	name = GetJString(env, splash_screen_name);
	if (name == NULL)
	{
		goto EXIT;
	}

	image = (*env)->NewByteArray(env, splash_screen_image_len);
	if (image == NULL)
	{
		goto EXIT;
	}
	(*env)->SetByteArrayRegion(env, image, 0, splash_screen_image_len, splash_screen_image_buf);

	exewrapClassLoader_setSplashScreenResource = (*env)->GetMethodID(env, ExewrapClassLoader, "setSplashScreenResource", "(Ljava/lang/String;[B)V");
	if (exewrapClassLoader_setSplashScreenResource == NULL)
	{
		goto EXIT;
	}

	(*env)->CallVoidMethod(env, exewrapClassLoader, exewrapClassLoader_setSplashScreenResource, name, image);

	ret = TRUE;

EXIT:
	if ((*env)->ExceptionCheck(env) == JNI_TRUE)
	{
		(*env)->ExceptionDescribe(env);
		(*env)->ExceptionClear(env);
		ret = FALSE;
	}
	return ret;
}


DWORD WINAPI RemoteCallMainMethod(void* _shared_memory_handle)
{
	HANDLE shared_memory_handle = (HANDLE)_shared_memory_handle;
	char* arglist;
	char* buf;
	int argc;
	char** argv = NULL;
	int i;
	LPSTR  shared_memory_read_event_name;
	HANDLE shared_memory_read_event_handle;
	LPBYTE lpShared = (LPBYTE)MapViewOfFile(shared_memory_handle, FILE_MAP_READ, 0, 0, 0);
	JNIEnv* env;
	jobjectArray args;

	arglist = (char*)(lpShared + sizeof(DWORD) + sizeof(DWORD));
	buf = (char*)HeapAlloc(GetProcessHeap(), 0, lstrlen(arglist) + 1);
	lstrcpy(buf, (char*)arglist);
	UnmapViewOfFile(lpShared);

	argv = split_args(buf, &argc);
	HeapFree(GetProcessHeap(), 0, buf);

	env = AttachJavaVM();
	if (env != NULL)
	{
		args = (*env)->NewObjectArray(env, argc - 1, (*env)->FindClass(env, "java/lang/String"), NULL);
		for (i = 1; i < argc; i++)
		{
			(*env)->SetObjectArrayElement(env, args, (i - 1), GetJString(env, argv[i]));
		}
	}

	shared_memory_read_event_name = GetModuleObjectName("SHARED_MEMORY_READ");
	shared_memory_read_event_handle = OpenEvent(EVENT_MODIFY_STATE, FALSE, shared_memory_read_event_name);
	if (shared_memory_read_event_handle != NULL)
	{
		SetEvent(shared_memory_read_event_handle);
		CloseHandle(shared_memory_read_event_handle);
	}
	HeapFree(GetProcessHeap(), 0, shared_memory_read_event_name);

	if (env == NULL)
	{
		goto EXIT;
	}

	(*env)->CallStaticVoidMethod(env, MainClass, MainClass_main, args);
	if ((*env)->ExceptionCheck(env) == JNI_TRUE)
	{
		print_stack_trace(NULL);
	}
	DetachJavaVM();

EXIT:
	if (argv != NULL)
	{
		HeapFree(GetProcessHeap(), 0, argv);
	}

	return 0;
}


char* ToString(JNIEnv* env, jobject object, char* buf)
{
	jclass    Object;
	jmethodID Object_getClass;
	jclass    object_class;
	jmethodID object_class_toString;
	jstring   str;
	char*     sjis;

	if (object == NULL)
	{
		return NULL;
	}
	Object = (*env)->FindClass(env, "java/lang/Object");
	if (Object == NULL)
	{
		return NULL;
	}
	Object_getClass = (*env)->GetMethodID(env, Object, "getClass", "()Ljava/lang/Class;");
	if (Object_getClass == NULL)
	{
		return NULL;
	}
	object_class = (*env)->CallObjectMethod(env, object, Object_getClass);
	if (object_class == NULL)
	{
		return NULL;
	}
	object_class_toString = (*env)->GetMethodID(env, object_class, "toString", "()Ljava/lang/String;");
	if (object_class_toString == NULL)
	{
		return NULL;
	}
	str = (*env)->CallObjectMethod(env, object, object_class_toString);
	if (str == NULL)
	{
		return NULL;
	}
	
	sjis = GetShiftJIS(env, str);
	if (buf != NULL)
	{
		strcpy(buf, sjis);
		HeapFree(GetProcessHeap(), 0, sjis);
		sjis = buf;
	}

	return sjis;
}


char* GetModuleObjectName(const char* prefix)
{
	char* object_name = (char*)HeapAlloc(GetProcessHeap(), 0, MAX_PATH + 32);
	char* module_filename = (char*)malloc(MAX_PATH);

	GetModuleFileName(NULL, module_filename, MAX_PATH);
	strcpy(object_name, "EXEWRAP:");
	if (prefix != NULL)
	{
		strcat(object_name, prefix);
	}
	strcat(object_name, ":");
	strcat(object_name, (char*)(strrchr(module_filename, '\\') + 1));

	free(module_filename);
	return object_name;
}


BYTE* GetResource(LPCTSTR name, RESOURCE* resource)
{
	HRSRC hrsrc;
	BYTE* buf = NULL;
	DWORD len = 0;

	if ((hrsrc = FindResource(NULL, name, RT_RCDATA)) != NULL)
	{
		buf = (BYTE*)LockResource(LoadResource(NULL, hrsrc));
		len = SizeofResource(NULL, hrsrc);
	}
	if (resource != NULL)
	{
		resource->buf = buf;
		resource->len = len;
	}
	return buf;
}


char* GetWinErrorMessage(DWORD err, int* exit_code, char* buf)
{
	LPVOID msg = NULL;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msg, 0, NULL);

	if (buf == NULL)
	{
		buf = (char*)HeapAlloc(GetProcessHeap(), 0, strlen(msg) + 1);
	}
	strcpy(buf, msg);
	LocalFree(msg);

	*exit_code = err;

	return buf;
}


char* GetJniErrorMessage(int err, int* exit_code, char* buf)
{
	switch (err) {
	case JNI_OK:
		*exit_code = 0;
		sprintf(buf, "");
		return NULL;
	case JNI_ERR: /* unknown error */
		*exit_code = MSG_ID_ERR_CREATE_JVM_UNKNOWN;
		strcpy(buf, _(MSG_ID_ERR_CREATE_JVM_UNKNOWN));
		break;
	case JNI_EDETACHED: /* thread detached from the VM */
		*exit_code = MSG_ID_ERR_CREATE_JVM_EDETACHED;
		strcpy(buf, _(MSG_ID_ERR_CREATE_JVM_EDETACHED));
		break;
	case JNI_EVERSION: /* JNI version error */
		*exit_code = MSG_ID_ERR_CREATE_JVM_EVERSION;
		strcpy(buf, _(MSG_ID_ERR_CREATE_JVM_EVERSION));
		break;
	case JNI_ENOMEM: /* not enough memory */
		*exit_code = MSG_ID_ERR_CREATE_JVM_ENOMEM;
		strcpy(buf, _(MSG_ID_ERR_CREATE_JVM_ENOMEM));
		break;
	case JNI_EEXIST: /* VM already created */
		*exit_code = MSG_ID_ERR_CREATE_JVM_EEXIST;
		strcpy(buf, _(MSG_ID_ERR_CREATE_JVM_EEXIST));
		break;
	case JNI_EINVAL: /* invalid arguments */
		*exit_code = MSG_ID_ERR_CREATE_JVM_EINVAL;
		strcpy(buf, _(MSG_ID_ERR_CREATE_JVM_EINVAL));
		break;
	case JVM_ELOADLIB:
		*exit_code = MSG_ID_ERR_CREATE_JVM_ELOADLIB;
		sprintf(buf, _(MSG_ID_ERR_CREATE_JVM_ELOADLIB), GetProcessArchitecture());
		break;
	default:
		*exit_code = MSG_ID_ERR_CREATE_JVM_UNKNOWN;
		strcpy(buf, _(MSG_ID_ERR_CREATE_JVM_UNKNOWN));
		break;
	}
	return buf;
}


void JNICALL JNI_WriteConsole(JNIEnv *env, jobject clazz, jbyteArray b, jint off, jint len)
{
	jboolean isCopy;
	BYTE*    buf;

	buf = (*env)->GetByteArrayElements(env, b, &isCopy);
	OutputConsole(buf, len);
	(*env)->ReleaseByteArrayElements(env, b, buf, 0);
}


void JNICALL JNI_WriteEventLog(JNIEnv *env, jobject clazz, jint logType, jstring message)
{
	WORD  nType;
	char* sjis;
	
	switch (logType)
	{
	case 0:  nType = EVENTLOG_INFORMATION_TYPE; break;
	case 1:  nType = EVENTLOG_WARNING_TYPE;     break;
	case 2:  nType = EVENTLOG_ERROR_TYPE;       break;
	default: nType = EVENTLOG_INFORMATION_TYPE;
	}
	sjis = GetShiftJIS(env, message);
	WriteEventLog(nType, sjis);
	HeapFree(GetProcessHeap(), 0, sjis);
}


void JNICALL JNI_UncaughtException(JNIEnv *env, jobject clazz, jstring thread, jstring message, jstring trace)
{
	char* sjis_thread;
	char* sjis_message;
	char* sjis_trace;
	UINT  exit_code;

	sjis_thread = GetShiftJIS(env, thread);
	sjis_message = GetShiftJIS(env, message);
	sjis_trace = GetShiftJIS(env, trace);

	exit_code = UncaughtException(sjis_thread, sjis_message, sjis_trace);

	if (sjis_thread != NULL)
	{
		HeapFree(GetProcessHeap(), 0, sjis_thread);
	}
	if (sjis_message != NULL)
	{
		HeapFree(GetProcessHeap(), 0, sjis_message);
	}
	if (sjis_trace != NULL)
	{
		HeapFree(GetProcessHeap(), 0, sjis_trace);
	}

	ExitProcess(exit_code);
}


jstring JNICALL JNI_SetEnvironment(JNIEnv *env, jobject clazz, jstring key, jstring value)
{
	char* sjis_key;
	char* sjis_value;
	DWORD size;
	char* sjis_buf = NULL;
	jstring prev_value = NULL;
	BOOL  result;

	sjis_key = GetShiftJIS(env, key);
	sjis_value = GetShiftJIS(env, value);

	size = GetEnvironmentVariable(sjis_key, NULL, 0);
	if (size > 0)
	{
		sjis_buf = (char*)malloc(size);
		size = GetEnvironmentVariable(sjis_key, sjis_buf, size);
		if (size > 0) {
			prev_value = GetJString(env, sjis_buf);
		}
		free(sjis_buf);
	}

	result = SetEnvironmentVariable(sjis_key, sjis_value);
	if (result == 0)
	{
		if (prev_value != NULL)
		{
			(*env)->DeleteLocalRef(env, prev_value);
			prev_value = NULL;
		}
	}

	return prev_value;
}


static jint register_native(JNIEnv* env, jclass cls, const char* name, const char* signature, void* fnPtr)
{
	JNINativeMethod nm;

	nm.name = (char*)name;
	nm.signature = (char*)signature;
	nm.fnPtr = fnPtr;

	return (*env)->RegisterNatives(env, cls, &nm, 1);
}

static void print_stack_trace(const char* text)
{
	jclass     Throwable;
	jthrowable throwable;
	jmethodID  throwable_toString;
	jstring    src;
	char*      message;

	if ((*env)->ExceptionCheck(env) != JNI_TRUE)
	{
		if (text != NULL)
		{
			OutputMessage(text);
		}
		goto EXIT;
	}

	Throwable = (*env)->FindClass(env, "java/lang/Throwable");

	throwable = (*env)->ExceptionOccurred(env);
	(*env)->ExceptionDescribe(env);
	(*env)->ExceptionClear(env);

	throwable_toString = (*env)->GetMethodID(env, Throwable, "toString", "()Ljava/lang/String;");

	src = (jstring)(*env)->CallObjectMethod(env, throwable, throwable_toString);
	message = GetShiftJIS(env, src);

	if (text != NULL)
	{
		char* buf = (char*)malloc(8192);
		strcpy(buf, text);
		strcat(buf, "\r\n");
		strcat(buf, message);
		OutputMessage(buf);
		free(buf);
	}
	else
	{
		OutputMessage(message);
	}

EXIT:
	if (message != NULL)
	{
		HeapFree(GetProcessHeap(), 0, message);
	}
}


static char** split_args(char* buffer, int* p_argc)
{
	int i;
	int buf_len = lstrlen(buffer);
	char** argv;

	*p_argc = 0;
	for (i = 0; i < buf_len; i++) {
		*p_argc += (buffer[i] == '\n') ? 1 : 0;
	}
	argv = (char**)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (*p_argc) * sizeof(char*));
	for (i = 0; i < *p_argc; i++)
	{
		argv[i] = strtok(i ? NULL : buffer, "\n");
	}
	return argv;
}
