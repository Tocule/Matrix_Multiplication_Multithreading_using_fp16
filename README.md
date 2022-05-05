# Matrix_Multiplication_Multithreading_using_fp16

The operation d=a*b+c is done on three matrices a,b and c using data format as fp16. First, the matrix is compressed from float to fp16 and then again 
decompressed to float to perform the operation. Multithreading has been used to speed up the matrix multiplication proces
