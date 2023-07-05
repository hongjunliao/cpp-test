/*!
 * easyexcel: https://codeload.github.com/alibaba/easyexcel/tar.gz/v2.2.3
 * JDK: apt install openjdk-8-jdk-headless
 * */
#ifdef CPP_TEST_WITH_JNI
#include<stdio.h>
#include "jni.h"
 
JNIEnv* create_vm(JavaVM **jvm)
{
    JNIEnv* env;
    JavaVMInitArgs args;
    JavaVMOption options;
    args.version = JNI_VERSION_1_6;
    args.nOptions = 1;
    options.optionString ="-Djava.class.path=./";
    args.options = &options;
    args.ignoreUnrecognized = 0;
    int rv;
    rv = JNI_CreateJavaVM(jvm,(void**)&env, &args);
    if (rv < 0 || !env)
        printf("Unable to Launch JVM%d\n",rv);
    else
        printf("Launched JVM! :)\n");
    return env;
}
 
 
void invoke_class(JNIEnv* env)
{
    jclass hello_world_class;
    jmethodID main_method;
    jmethodID square_method;
    jmethodID power_method;
    jint number=20;
    jint exponent=3;
    hello_world_class =(*env)->FindClass(env, "HelloWorld");
    main_method =(*env)->GetStaticMethodID(env, hello_world_class, "main","([Ljava/lang/String;)V");
    square_method =(*env)->GetStaticMethodID(env, hello_world_class, "square","(I)I");
    power_method =(*env)->GetStaticMethodID(env, hello_world_class, "power","(II)I");
    (*env)->CallStaticVoidMethod(env,hello_world_class, main_method, NULL);
    printf("%d squared is %d\n",number,
        (*env)->CallStaticIntMethod(env,hello_world_class, square_method, number));
    printf("%d raised to the %d power is%d\n", number, exponent,
        (*env)->CallStaticIntMethod(env,hello_world_class, power_method, number, exponent));
}
 
 
int main(int argc,char **argv)
{
    JavaVM *jvm;
    JNIEnv *env;
    env = create_vm(&jvm);
    if(env == NULL)
        return 1;
    invoke_class(env);
    return 0;
}
#endif //
