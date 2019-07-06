
tests: test1 test2

test1:
	gcc test_cameraCapture.c ../src/cameraCapture.c -o cameraCapture.out
	./cameraCapture.out
	@rm ./*.out
	@rm ./*.raw

test2:
	gcc test_frameBufferWriter.c ../src/frameBufferWriter.c -o test_frameBufferWriter.out
	./test_frameBufferWriter.out
	@rm ./*.out

test3:
	gcc test_imageWrite2fb0.c ../src/frameBufferWriter.c -o test_imageWrite2fb0.out
	./test_imageWrite2fb0.out
	@rm ./*.out
