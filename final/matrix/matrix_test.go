package matrix

import (
	"testing"
	"time"
)

func TestGet(t *testing.T) {
	elements := []float64{1, 1, 0, 1}
	A := NewMatrix(2, 2, elements)
	v := A.Get(1, 0)
	if v != 0 {
		t.Fail()
	}
}

func TestSet(t *testing.T) {
	A := ZeroedMatrix(2, 2)
	A.Set(1, 1, 5)
	v := A.Get(1, 1)
	if v != 5 {
		t.Fail()
	}
}

func TestEquals(t *testing.T) {
	A := ZeroedMatrix(2, 2)
	B := ZeroedMatrix(2, 2)

	if !Equals(A, B) {
		t.Logf("Does not equal self.")
		t.Fail()
	}

	B = ZeroedMatrix(2, 3)
	if Equals(A, B) {
		t.Logf("Equals invalid col.")
		t.Fail()
	}

	B = ZeroedMatrix(3, 2)
	if Equals(A, B) {
		t.Logf("Equals invalid row.")
		t.Fail()
	}

	B = ZeroedMatrix(3, 3)
	if Equals(A, B) {
		t.Logf("Equals invalid row & col.")
		t.Fail()
	}

	B = NewMatrix(2, 2, []float64{1, 2, 3, 4})
	if Equals(A, B) {
		t.Logf("Equals non-equal elements")
		t.Fail()
	}
}

/*
 * Math Tests
 */

type multTest struct {
	A        *Matrix
	B        *Matrix
	expected *Matrix
}

var multTests = []multTest{
	{
		ZeroedMatrix(2, 2),
		ZeroedMatrix(2, 2),
		ZeroedMatrix(2, 2),
	},
	{
		ZeroedMatrix(2, 2),
		NewMatrix(2, 2, []float64{1, 2, 3, 4}),
		ZeroedMatrix(2, 2),
	},
	{
		NewMatrix(2, 2, []float64{1, 2, 3, 4}),
		NewMatrix(2, 2, []float64{1, 1, 1, 1}),
		NewMatrix(2, 2, []float64{3, 3, 7, 7}),
	},
	{
		NewMatrix(2, 2, []float64{1, 2, 3, 4}),
		NewMatrix(2, 2, []float64{1, 2, 3, 4}),
		NewMatrix(2, 2, []float64{7, 10, 15, 22}),
	},
	{
		NewMatrix(2, 2, []float64{1, 2, 3, 4}),
		NewMatrix(2, 2, []float64{1, 0, 0, 1}),
		NewMatrix(2, 2, []float64{1, 2, 3, 4}),
	},
	{
		NewMatrix(8, 8, []float64{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64}),
		NewMatrix(8, 8, []float64{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64}),
		NewMatrix(8, 8, []float64{1380, 1416, 1452, 1488, 1524, 1560, 1596, 1632, 3236, 3336, 3436, 3536, 3636, 3736, 3836, 3936, 5092, 5256, 5420, 5584, 5748, 5912, 6076, 6240, 6948, 7176, 7404, 7632, 7860, 8088, 8316, 8544, 8804, 9096, 9388, 9680, 9972, 10264, 10556, 10848, 10660, 11016, 11372, 11728, 12084, 12440, 12796, 13152, 12516, 12936, 13356, 13776, 14196, 14616, 15036, 15456, 14372, 14856, 15340, 15824, 16308, 16792, 17276, 17760}),
	},
}

func TestMultiplySerial(t *testing.T) {
	for index, test := range multTests {
		if !Equals(test.A.MultiplySerial(test.B), test.expected) {
			t.Logf("Case %d failed.", index)
			t.Fail()
		}
	}
}

func TestMultiplyParallelNaive(t *testing.T) {
	for index, test := range multTests {
		if !Equals(test.A.MultiplyParallelNaive(test.B), test.expected) {
			t.Logf("Case %d failed.", index)
			t.Fail()
		}
	}
}

func TestMultiplyParallelRecursive(t *testing.T) {
	for index, test := range multTests {
		if !Equals(test.A.MultiplyParallelRecursive(test.B), test.expected) {
			t.Logf("Case %d failed.", index)
			t.Fail()
		}
	}
}

/*
 * Benchmarks
 */

var benchMatrices map[int]*Matrix

func benchmarkSetup() {
	if benchMatrices == nil {
		benchMatrices = map[int]*Matrix{
			100:  RandomMatrix(1, 1),
			200:  RandomMatrix(2, 2),
			300:  RandomMatrix(4, 4),
			400:  RandomMatrix(400, 400),
			500:  RandomMatrix(500, 500),
			600:  RandomMatrix(600, 600),
			700:  RandomMatrix(700, 700),
			800:  RandomMatrix(800, 800),
			900:  RandomMatrix(900, 900),
			1000: RandomMatrix(1000, 1000),
		}
	}
}

var result *Matrix

func benchmarkMultiplySerial(i int, b *testing.B) {
	benchmarkSetup()
	mat := benchMatrices[i]
	var C *Matrix
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		// Record the result to prevent compiler optimization.
		C = mat.MultiplySerial(mat)
	}
	// Store the result in a package variable, again, to prevent
	// the compiler from optimizing away the function call.
	result = C
}

func BenchmarkMultiplySerial100(b *testing.B) {
	benchmarkMultiplySerial(100, b)
}
func BenchmarkMultiplySerial200(b *testing.B) {
	benchmarkMultiplySerial(200, b)
}
func BenchmarkMultiplySerial300(b *testing.B) {
	benchmarkMultiplySerial(300, b)
}
func BenchmarkMultiplySerial400(b *testing.B) {
	benchmarkMultiplySerial(500, b)
}
func BenchmarkMultiplySerial500(b *testing.B) {
	benchmarkMultiplySerial(500, b)
}
func BenchmarkMultiplySerial600(b *testing.B) {
	benchmarkMultiplySerial(600, b)
}
func BenchmarkMultiplySerial700(b *testing.B) {
	benchmarkMultiplySerial(700, b)
}
func BenchmarkMultiplySerial800(b *testing.B) {
	benchmarkMultiplySerial(800, b)
}
func BenchmarkMultiplySerial900(b *testing.B) {
	benchmarkMultiplySerial(900, b)
}
func BenchmarkMultiplySerial1000(b *testing.B) {
	benchmarkMultiplySerial(1000, b)
}

func benchmarkMultiplyParallelNaive(i int, b *testing.B) {
	benchmarkSetup()
	mat := benchMatrices[i]
	var C *Matrix
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		// Record the result to prevent compiler optimization
		C = mat.MultiplyParallelNaive(mat)
	}
	// Store the result in a package variable, again, to prevent
	// the compiler from optimizing away the function call.
	result = C
}

func BenchmarkMultiplyParallelNaive100(b *testing.B) {
	benchmarkMultiplyParallelNaive(100, b)
}
func BenchmarkMultiplyParallelNaive200(b *testing.B) {
	benchmarkMultiplyParallelNaive(200, b)
}
func BenchmarkMultiplyParallelNaive300(b *testing.B) {
	benchmarkMultiplyParallelNaive(300, b)
}
func BenchmarkMultiplyParallelNaive400(b *testing.B) {
	benchmarkMultiplyParallelNaive(400, b)
}
func BenchmarkMultiplyParallelNaive500(b *testing.B) {
	benchmarkMultiplyParallelNaive(500, b)
}
func BenchmarkMultiplyParallelNaive600(b *testing.B) {
	benchmarkMultiplyParallelNaive(600, b)
}
func BenchmarkMultiplyParallelNaive700(b *testing.B) {
	benchmarkMultiplyParallelNaive(700, b)
}
func BenchmarkMultiplyParallelNaive800(b *testing.B) {
	benchmarkMultiplyParallelNaive(800, b)
}
func BenchmarkMultiplyParallelNaive900(b *testing.B) {
	benchmarkMultiplyParallelNaive(900, b)
}
func BenchmarkMultiplyParallelNaive1000(b *testing.B) {
	benchmarkMultiplyParallelNaive(1000, b)
}

func benchmarkMultiplyParallelRecursive(i int, b *testing.B) {
	benchmarkSetup()
	mat := benchMatrices[i]
	var C *Matrix
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		// Record the result to prevent compiler optimization
		C = mat.MultiplyParallelRecursive(mat)
	}
	// Store the result in a package variable, again, to prevent
	// the compiler from optimizing away the function call.
	result = C
}

func BenchmarkMultiplyParallelRecursive100(b *testing.B) {
	benchmarkMultiplyParallelRecursive(100, b)
}
func BenchmarkMultiplyParallelRecursive200(b *testing.B) {
	benchmarkMultiplyParallelRecursive(200, b)
}
func BenchmarkMultiplyParallelRecursive300(b *testing.B) {
	benchmarkMultiplyParallelRecursive(300, b)
}
func BenchmarkMultiplyParallelRecursive400(b *testing.B) {
	benchmarkMultiplyParallelRecursive(400, b)
}
func BenchmarkMultiplyParallelRecursive500(b *testing.B) {
	benchmarkMultiplyParallelRecursive(500, b)
}
func BenchmarkMultiplyParallelRecursive600(b *testing.B) {
	benchmarkMultiplyParallelRecursive(600, b)
}
func BenchmarkMultiplyParallelRecursive700(b *testing.B) {
	benchmarkMultiplyParallelRecursive(700, b)
}
func BenchmarkMultiplyParallelRecursive800(b *testing.B) {
	benchmarkMultiplyParallelRecursive(800, b)
}
func BenchmarkMultiplyParallelRecursive900(b *testing.B) {
	benchmarkMultiplyParallelRecursive(900, b)
}
func BenchmarkMultiplyParallelRecursive1000(b *testing.B) {
	benchmarkMultiplyParallelRecursive(1000, b)
}

func BenchmarkThreshold(b *testing.B) {
	A := RandomMatrix(500, 500)
	B := RandomMatrix(500, 500)
	var start, end time.Time
	var duration float64

	b.Logf("Benchmarking threshold values.")
	// It'll only print 9 or so logs, so range was progressively
	// narrowed.
	for i := 20; i < 100; i += 10 {
		start = time.Now()
		for j := 0; j < 50; j++ {
			A.MultiplyParallelRecursiveThreshold(B, i)
		}
		end = time.Now()
		duration = float64(end.Sub(start)) / 1e9
		b.Logf("%d: \t%f", i, duration)
	}
}
