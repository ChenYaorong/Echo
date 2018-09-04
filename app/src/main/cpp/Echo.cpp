// JNI
#include <jni.h>
// NULL
#include <stdio.h>
// va_list, vsnprintf
// errno
#include <errno.h>
// strerror_r, memset
#include <string.h>
// socket, bind, getsockname, listen, accept, recv, send, connect
#include <sys/types.h>
#include <sys/socket.h>
// sockaddr_un
// htons, sockaddr_in
#include <netinet/in.h>
// inet_ntop
#include <arpa/inet.h>
// close, unlink
#include <unistd.h>

#include <string>

#include <sys/endian.h>
// offsetof
#include <stddef.h>
// Max log message length
#define MAX_LOG_MESSAGE_LENGTH 256
// Max data buffer size
#define MAX_BUFFER_SIZE 80

/**
 * Logs the given message to the application.
 *
 * @param env JNIEnv interface.
 * @param obj java object instance
 */
void LogMessage(JNIEnv *env, jobject obj, const char *format, ...);

/**
* Throws a new exception using the given exception class
* and exception message.
*
* @param env JNIEnv interface.
* @param className class name.
* @param message exception message.
*/
static void ThrowException(JNIEnv *env, const char *className, const char *message);

/**
* Throws a new exception using the given exception class
* and error message based on the error number.
*
* @param env JNIEnv interface.
* @param className class name.
* @param errnum error number.
*/
static void ThrowErrnoException(JNIEnv *env, const char *className, int errnum);

/**
 * creates a new TCP socket
 *
 * @param env env JNIEnv interface.
 * @param obj java object instance
 * @return the associated socket descriptor
 */
static int NewTcpSocket(JNIEnv *env, jobject obj);

/**
* Binds socket to a port number.
*
* @param env JNIEnv interface.
* @param obj object instance.
* @param sd socket descriptor.
* @param port port number or zero for random port.
* @throws IOException
*/
static void BindSocketToPort(JNIEnv *env, jobject obj, int sd, unsigned short port);

/**
* Gets the port number socket is currently binded.
*
* @param env JNIEnv interface.
* @param obj object instance.
* @param sd socket descriptor.
* @return port number.
* @throws IOException
*/
static unsigned short GetSocketPort(JNIEnv *env, jobject obj, int sd);

/**
* Listens on given socket with the given backlog for
* pending connections. When the backlog is full, the
* new connections will be rejected.
*
* @param env JNIEnv interface.
* @param obj object instance.
* @param sd socket descriptor.
* @param backlog backlog size.
* @throws IOException
*/
static void ListenOnSocket(JNIEnv *env, jobject obj, int sd, int backlog);

/**
* Logs the IP address and the port number from the
* given address.
*
* @param env JNIEnv interface.
* @param obj object instance.
* @param message message text.
* @param address adress instance.
* @throws IOException
*/
static void LogAddress(JNIEnv *env, jobject obj, const char *message,
                       const struct sockaddr_in *address);

/**
* Blocks and waits for incoming client connections on the
* given socket.
*
* @param env JNIEnv interface.
* @param obj object instance.
* @param sd socket descriptor.
* @return client socket.
* @throws IOException
*/
static int AcceptOnSocket(JNIEnv *env, jobject obj, int sd);

extern "C" JNIEXPORT jstring
JNICALL
Java_com_apress_echo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_apress_echo_EchoServerActivity_nativeStartTcpServer(JNIEnv *env, jobject instance,
                                                             jint port) {

    // TODO

}extern "C"
JNIEXPORT void JNICALL
Java_com_apress_echo_EchoServerActivity_nativeStartUdpServer(JNIEnv *env, jobject instance,
                                                             jint port) {

    // TODO

}


void LogMessage(JNIEnv *env, jobject obj, const char *format, ...) {
    // Cached log method ID
    static jmethodID methodID = NULL;
    // If method ID is not cached
    if (NULL == methodID) {
        // Get class from object
        jclass clazz = env->GetObjectClass(obj);
        // Get the method ID for the given method
        methodID = env->GetMethodID(clazz, "logMessage",
                                    "(Ljava/lang/String;)V");
        // Release the class reference
        env->DeleteLocalRef(clazz);
    }
    // If method is found
    if (NULL != methodID) {
        // Format the log message
        char buffer[MAX_LOG_MESSAGE_LENGTH];
        va_list ap;
        va_start(ap, format);
        vsnprintf(buffer, MAX_LOG_MESSAGE_LENGTH, format, ap);
        va_end(ap);
        // Convert the buffer to a Java string
        jstring message = env->NewStringUTF(buffer);
        // If string is properly constructed
        if (NULL != message) {
            // Log message
            env->CallVoidMethod(obj, methodID, message);
            // Release the message reference
            env->DeleteLocalRef(message);
        }
    }
}

static void ThrowException(JNIEnv *env, const char *className, const char *message) {
    // Get the exception class
    jclass clazz = env->FindClass(className);
    // If exception class is found
    if (NULL != clazz) {
        // Throw exception
        env->ThrowNew(clazz, message);
        // Release local class reference
        env->DeleteLocalRef(clazz);
    }
}

static void ThrowErrnoException(JNIEnv *env, const char *className, int errnum) {
    char buffer[MAX_LOG_MESSAGE_LENGTH];
    // Get message for the error number
    if (-1 == strerror_r(errnum, buffer, MAX_LOG_MESSAGE_LENGTH)) {
        strerror_r(errno, buffer, MAX_LOG_MESSAGE_LENGTH);
    }
    // Throw exception
    ThrowException(env, className, buffer);
}

static int NewTcpSocket(JNIEnv *env, jobject obj) {
    // Construct socket
    LogMessage(env, obj, "Constructing a new TCP socket...");
    int tcpSocket = socket(PF_INET, SOCK_STREAM, 0);
    // Check if socket is properly constructed
    if (-1 == tcpSocket) {
        // Throw an exception with error number
        ThrowErrnoException(env, "java/io/IOException", errno);
    }
    return tcpSocket;
}

static void BindSocketToPort(JNIEnv *env, jobject obj, int sd, unsigned short port) {
    struct sockaddr_in address;
    // Address to bind socket
    memset(&address, 0, sizeof(address));
    address.sin_family = PF_INET;
    // Bind to all addresses
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    // Convert port to network byte order
    address.sin_port = htons(port);
    // Bind socket
    LogMessage(env, obj, "Binding to port %hu.", port);
    if (-1 == bind(sd, (struct sockaddr *) &address, sizeof(address))) {
        // Throw an exception with error number
        ThrowErrnoException(env, "java/io/IOException", errno);
    }
}

static unsigned short GetSocketPort(JNIEnv *env, jobject obj, int sd) {
    unsigned short port = 0;
    struct sockaddr_in address;
    socklen_t addressLength = sizeof(address);
    // Get the socket address
    if (-1 == getsockname(sd, (struct sockaddr *) &address, &addressLength)) {
        // Throw an exception with error number
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        // Convert port to host byte order
        port = ntohs(address.sin_port);
        LogMessage(env, obj, "Binded to random port %hu.", port);
    }
    return port;
}

static void ListenOnSocket(JNIEnv *env, jobject obj, int sd, int backlog) {
    // Listen on socket with the given backlog
    LogMessage(env, obj, "Listening on socket with a backlog of %d pending connections.", backlog);
    if (-1 == listen(sd, backlog)) {
        // Throw an exception with error number
        ThrowErrnoException(env, "java/io/IOException", errno);
    }
}

static void LogAddress(JNIEnv *env, jobject obj, const char *message,
                       const struct sockaddr_in *address) {
    char ip[INET_ADDRSTRLEN];
    // Convert the IP address to string
    if (NULL == inet_ntop(PF_INET, &(address->sin_addr), ip, INET_ADDRSTRLEN)) {
        // Throw an exception with error number
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        // Convert port to host byte order
        unsigned short port = ntohs(address->sin_port);
        // Log address
        LogMessage(env, obj, "%s %s:%hu.", message, ip, port);
    }
}

static int AcceptOnSocket(JNIEnv *env, jobject obj, int sd) {
    struct sockaddr_in address;
    socklen_t addressLength = sizeof(address);
    // Blocks and waits for an incoming client connection
    // and accepts it
    LogMessage(env, obj, "Waiting for a client connection...");
    int clientSocket = accept(sd, (struct sockaddr *) &address, &addressLength);
    // If client socket is not valid
    if (-1 == clientSocket) {
        // Throw an exception with error number
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        // Log address
        LogAddress(env, obj, "Client connection from ", &address);
    }
    return clientSocket;
}