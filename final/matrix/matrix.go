package matrix

import (
	"fmt"
	"math/rand"
)

type Matrix struct {
	rows     int
	cols     int
	elements []float64
}

func Equals(A, B *Matrix) bool {
	if A.Rows() != B.Rows() || A.Cols() != B.Cols() {
		return false
	}
	for i := 0; i < A.Rows(); i++ {
		for j := 0; j < A.Cols(); j++ {
			if A.Get(i, j) != B.Get(i, j) {
				return false
			}
		}
	}
	return true
}

func (A *Matrix) Nil() bool {
	return A == nil
}

func (A *Matrix) Rows() int {
	return A.rows
}

func (A *Matrix) Cols() int {
	return A.cols
}

func (A *Matrix) Size() (int, int) {
	return A.rows, A.cols
}

func (A *Matrix) Get(row, col int) float64 {
	return A.elements[row*A.cols+col]
}

func (A *Matrix) Set(row, col int, val float64) {
	A.elements[row*A.cols+col] = val
}

func NewMatrix(rows, cols int, elements []float64) *Matrix {
	A := new(Matrix)
	A.rows = rows
	A.cols = cols
	A.elements = elements
	return A
}

func ZeroedMatrix(rows, cols int) *Matrix {
	return NewMatrix(rows, cols, make([]float64, rows*cols))
}

func RandomMatrix(rows, cols int) *Matrix {
	size := rows * cols
	elements := make([]float64, size)

	for i := 0; i < size; i++ {
		elements[i] = rand.Float64()
	}

	return NewMatrix(rows, cols, elements)
}

func (A *Matrix) GetSubmatrix(row, col, rows, cols int) *Matrix {
	B := new(Matrix)
	B.elements = A.elements[row*A.cols+col : row*A.cols+col+(rows-1)*A.cols+cols]
	B.rows = rows
	B.cols = cols
	return B
}

func (A *Matrix) GetRow(row int) *Matrix {
	return A.GetSubmatrix(row, 0, 1, A.cols)
}

func (A *Matrix) GetCol(col int) *Matrix {
	return A.GetSubmatrix(0, col, A.rows, 1)
}

func (A *Matrix) String() string {
	var str = "{\n "
	for i := 0; i < A.Rows(); i++ {
		if i != 0 {
			str += " "
		}
		for j := 0; j < A.Cols(); j++ {
			val := A.Get(i, j)
			valString := fmt.Sprintf("%f", val)
			str += valString
			if j != A.Cols()-1 {
				str += ", "
			}
		}
		str += "\n"
	}
	str += "}"
	return str
}
