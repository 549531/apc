#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <cstdio>

int main() {
	cl_int CL_err = CL_SUCCESS;
	cl_uint numPlatforms = 0;

	CL_err = clGetPlatformIDs(0, NULL, &numPlatforms);

	if (CL_err != CL_SUCCESS) {
		printf("clGetPlatformIDs(%i)\n", CL_err);
		return 1;
	}

	printf("%u platform(s) found\n", numPlatforms);
}
