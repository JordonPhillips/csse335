package matrix

func (A *Matrix) MultiplySerial(B *Matrix) (C *Matrix) {
	C = ZeroedMatrix(A.rows, B.cols)
	for i := 0; i < A.rows; i++ {
		MultiplyRowSerial(A, B, C, i)
	}
	return C
}

func MultiplyRowSerial(A, B, C *Matrix, row int) {
	temp := C.elements[row*C.cols : (row+1)*C.cols]
	for k, a := range A.elements[row*A.cols : (row+1)*A.cols] {
		for j, b := range B.elements[k*B.cols : (k+1)*B.cols] {
			temp[j] += a * b
		}
	}
}

func (A *Matrix) MultiplyParallelNaive(B *Matrix) (C *Matrix) {
	C = ZeroedMatrix(A.rows, B.cols)
	done := make(chan bool)
	for i := 0; i < A.rows; i++ {
		go func(i int) {
			MultiplyRowSerial(A, B, C, i)
			done <- true
		}(i)
	}

	for i := 0; i < A.rows; i++ {
		<-done
	}
	return C
}

func (A *Matrix) MultiplyParallelRecursive(B *Matrix) (C *Matrix) {
	done := make(chan bool)
	C = ZeroedMatrix(A.rows, B.cols)
	go multiplyParallelRecursive(A, B, C, 0, A.rows, 0, B.cols, 0, A.cols, 60, done)
	<-done
	return C
}

func (A *Matrix) MultiplyParallelRecursiveThreshold(B *Matrix, threshold int) (C *Matrix) {
	done := make(chan bool)
	C = ZeroedMatrix(A.rows, B.cols)
	go multiplyParallelRecursive(A, B, C, 0, A.rows, 0, B.cols, 0, A.cols, threshold, done)
	<-done
	return C
}

func multiplyParallelRecursive(A, B, C *Matrix, firstRow, lastRow, firstCol, lastCol, firstK, lastK, threshold int, done chan bool) {
	di := lastRow - firstRow
	dj := lastCol - firstCol
	dk := lastK - firstK
	done2 := make(chan bool)

	switch {
	case di >= dj && di >= dk && di >= threshold:
		middleRow := firstRow + di/2
		go multiplyParallelRecursive(A, B, C, firstRow, middleRow, firstCol, lastCol, firstK, lastK, threshold, done2)
		multiplyParallelRecursive(A, B, C, middleRow, lastRow, firstCol, lastCol, firstK, lastK, threshold, nil)
		<-done2
	case dj >= dk && dj >= threshold:
		middleCol := firstCol + dj/2
		go multiplyParallelRecursive(A, B, C, firstRow, lastRow, firstCol, middleCol, firstK, lastK, threshold, done2)
		multiplyParallelRecursive(A, B, C, firstRow, lastRow, middleCol, lastCol, firstK, lastK, threshold, nil)
		<-done2
	case dk >= threshold:
		middleK := firstK + dk/2
		go multiplyParallelRecursive(A, B, C, firstRow, lastRow, firstCol, lastCol, firstK, middleK, threshold, done2)
		multiplyParallelRecursive(A, B, C, firstRow, lastRow, firstCol, lastCol, middleK, lastK, threshold, nil)
		<-done2
	default:
		for i := firstRow; i < lastRow; i++ {
			for j := firstCol; j < lastCol; j++ {
				for k := firstK; k < lastK; k++ {
					C.elements[i*C.cols+j] += A.elements[i*A.cols+k] * B.elements[k*B.cols+j]
				}
			}
		}
	}

	if done != nil {
		done <- true
	}
}
