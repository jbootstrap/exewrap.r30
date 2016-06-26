#include <windows.h>
#include <locale.h>
#include <mbctype.h>
#include "include/message.h"

static const WORD LANGID_ja_JP = MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);

const char* get_message(int msg_id);
static int  get_lang_index();
static void init_message();
static int  lang_index = -1;
static const char** msg;

const char* get_message(int msg_id)
{
	if(lang_index < 0)
	{
		lang_index = get_lang_index();
		init_message();
	}

	return msg[MSG_ID_COUNT * lang_index + msg_id];
}

static int get_lang_index()
{
	int codepage = GetConsoleCP();

	switch(codepage)
	{
	case 65001: return MSG_LANG_ID_EN;
	case 437:   return MSG_LANG_ID_EN;
	case 932:   return MSG_LANG_ID_JA;
	}

	if(GetUserDefaultLangID() == LANGID_ja_JP)
	{
		return MSG_LANG_ID_JA;
	}

	return MSG_LANG_ID_EN;
}

static void init_message()
{
	int size = MSG_LANG_ID_COUNT * (MSG_ID_COUNT + 1) * sizeof(const char*);

	msg = (const char**)HeapAlloc(GetProcessHeap(), 0, size);
	SecureZeroMemory((void*)msg, size);

	//
	// ENGLISH
	//
	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_CREATE_JVM_UNKNOWN] =
	"Failed to create the Java Virtual Machine. An unknown error has occurred.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_CREATE_JVM_EDETACHED] =
	"Failed to create the Java Virtual Machine. Thread detached from the Java Virtual Machine.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_CREATE_JVM_EVERSION] =
	"Failed to create the Java Virtual Machine. JNI version error.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_CREATE_JVM_ENOMEM] =
	"Failed to create the Java Virtual Machine. Not enough memory available to create the Java Virtual Machine.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_CREATE_JVM_EEXIST] =
	"Failed to create the Java Virtual Machine. The Java Virtual Machine already exists.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_CREATE_JVM_EINVAL] =
	"Failed to create the Java Virtual Machine. An invalid argument was supplied.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_CREATE_JVM_ELOADLIB] =
	"Failed to create the Java Virtual Machine. Failed to load jvm.dll.\n"
	"%d-bit Java Virtual Machine required.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_UNCAUGHT_EXCEPTION] =
	"Uncaught exception occured.\n";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_RESOURCE_NOT_FOUND] =
	"Resource not found: %s";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_TARGET_VERSION] =
	"Java %s or higher is required to run this program.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_DEFINE_CLASS] =
	"Class not found: %s";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_GET_CONSTRUCTOR] =
	"Constructor not found: %s";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_GET_METHOD] =
	"Method not found: %s";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_NEW_OBJECT] =
	"Failed to create object: %s";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_FIND_CLASSLOADER] =
	"Class Loader not found.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_REGISTER_NATIVE] =
	"Failed to register native methods: %s";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_LOAD_MAIN_CLASS] =
	"Failed to load the Main Class.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_FIND_MAIN_CLASS] =
	"Main class not found.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_FIND_MAIN_METHOD] =
	"Main method is not implemented.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_FIND_METHOD_SERVICE_START] =
	"The service could not be started. Requires to define a method within main class: public static void start(String[] args)";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_FIND_METHOD_SERVICE_STOP] =
	"The service could not be started. Requires to define a method within main class: public static void stop()";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_SERVICE_ABORT] =
	"%s service was terminated abnormally.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_ERR_SERVICE_NOT_STOPPED] =
	"%s service can't be deleted because it is not stopped.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_SUCCESS_SERVICE_INSTALL] =
	"%s service installed.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_SUCCESS_SERVICE_REMOVE] =
	"%s service was removed.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_SERVICE_STARTING] =
	"%s service is starting.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_SERVICE_STOPING] =
	"%s service is stopping.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_SUCCESS_SERVICE_START] =
	"The %s service was started successfully.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_SUCCESS_SERVICE_STOP] =
	"The %s service was stopped successfully.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_CTRL_SERVICE_STOP] =
	"%s: The service is stopping.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_CTRL_SERVICE_TERMINATE] =
	"%s: The service is terminating.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_EN + MSG_ID_CTRL_BREAK] =
	"%s: ";

	//
	// JAPANESE
	//
	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_CREATE_JVM_UNKNOWN] =
	"JavaVM���쐬�ł��܂���ł����B�s���ȃG���[���������܂����B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_CREATE_JVM_EDETACHED] =
	"JavaVM���쐬�ł��܂���ł����B�X���b�h��JavaVM�ɐڑ�����Ă��܂���B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_CREATE_JVM_EVERSION] =
	"JavaVM���쐬�ł��܂���ł����B�T�|�[�g����Ă��Ȃ��o�[�W�����ł��B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_CREATE_JVM_ENOMEM] =
	"JavaVM���쐬�ł��܂���ł������B���������s�����Ă��܂��B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_CREATE_JVM_EEXIST] =
	"JavaVM���쐬�ł��܂���ł����BJavaVM�͊��ɍ쐬����Ă��܂��B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_CREATE_JVM_EINVAL] =
	"JavaVM���쐬�ł��܂���ł����B�������s���ł��B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_CREATE_JVM_ELOADLIB] =
	"JavaVM���쐬�ł��܂���ł����Bjvm.dll�����[�h�ł��܂���ł����B\n"
	"���̃v���O�����̎��s�ɂ� %d-bit JavaVM ���K�v�ł��B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_UNCAUGHT_EXCEPTION] =
	"�⑫����Ȃ���O���������܂����B.\n";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_RESOURCE_NOT_FOUND] =
	"���\�[�X��������܂���: %s";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_TARGET_VERSION] =
	"���̃v���O�����̎��s�ɂ� Java %s �ȏオ�K�v�ł��B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_DEFINE_CLASS] =
	"�N���X��������܂���: %s";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_GET_CONSTRUCTOR] =
	"�R���X�g���N�^�[��������܂���: %s";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_GET_METHOD] =
	"���\�b�h��������܂���: %s";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_NEW_OBJECT] =
	"�I�u�W�F�N�g�̍쐬�Ɏ��s���܂���: %s";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_FIND_CLASSLOADER] =
	"�N���X���[�_�[��������܂���B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_REGISTER_NATIVE] =
	"�l�C�e�B�u���\�b�h�̓o�^�Ɏ��s���܂���: %s";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_LOAD_MAIN_CLASS] =
	"���C���N���X�̃��[�h�Ɏ��s���܂����B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_FIND_MAIN_CLASS] =
	"���C���N���X��������܂���B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_FIND_MAIN_METHOD] =
	"main���\�b�h����������Ă��܂���B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_FIND_METHOD_SERVICE_START] =
	"�T�[�r�X���J�n�ł��܂���BWindows�T�[�r�X�́A���C���N���X�� public static void start(String[] args) ����������K�v������܂��B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_FIND_METHOD_SERVICE_STOP] =
	"�T�[�r�X���J�n�ł��܂���BWindows�T�[�r�X�́A���C���N���X�� public static void stop() ����������K�v������܂��B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_SERVICE_ABORT] =
	"%s �T�[�r�X�ُ͈�I�����܂����B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_ERR_SERVICE_NOT_STOPPED] =
	"%s �T�[�r�X����~���Ă��Ȃ����߁A�폜�ł��܂���B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_SUCCESS_SERVICE_INSTALL] =
	"%s �T�[�r�X���C���X�g�[�����܂����B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_SUCCESS_SERVICE_REMOVE] =
	"%s �T�[�r�X���폜���܂����B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_SERVICE_STARTING] =
	"%s �T�[�r�X���J�n���܂�.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_SERVICE_STOPING] =
	"%s �T�[�r�X���~���ł�.";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_SUCCESS_SERVICE_START] =
	"%s �T�[�r�X�͐���ɊJ�n����܂����B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_SUCCESS_SERVICE_STOP] =
	"%s �T�[�r�X�͐���ɒ�~����܂����B";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_CTRL_SERVICE_STOP] =
	"%s: �T�[�r�X���~���ł�...";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_CTRL_SERVICE_TERMINATE] =
	"%s: �����I�����܂�...";

	msg[MSG_ID_COUNT * MSG_LANG_ID_JA + MSG_ID_CTRL_BREAK] =
	"%s: ";
}
