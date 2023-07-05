#ifdef CPP_TEST_WITH_OPENGL

/*!
 * test for opengl
 * @author hongjun.liao <docici@126.com>, @date 2016/05/04
 *
 * install: freeglut3-dev
 * link:    -lglut
 * */
#include <stdio.h>
#if 0

#include <GL/glut.h>

void myDisplay(void)
{
     glClear(GL_COLOR_BUFFER_BIT);
     glRectf(0, 0, 1, 1);
     glFlush();
}

int test_opengl_main(int argc, char ** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 600);
    glutCreateWindow("hello_opengl");
    glutDisplayFunc(&myDisplay);
    glutMainLoop();
    return 0;
}
#else
int test_opengl_main(int argc, char ** argv)
{
	printf("%s: NOT implement yet!\n", __FUNCTION__);
	return -1;
}
#endif

#endif

