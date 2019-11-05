#include <random>
#include <cassert>
#include <iostream>

/*
desc: I1 cache:         32768 B, 64 B, 8-way associative
desc: D1 cache:         32768 B, 64 B, 8-way associative
  aka 32K
desc: LL cache:         8388608 B, 64 B, 16-way associative
  aka 8M
*/

extern "C" void sgemm_(char *transa, char *transb, int *m, int *n, int *k, float *alpha, float *a, int *lda, float *b, int *ldb, float *beta, float *c, int *ldc);

#if 0
// A little less big than a full L1 cache
constexpr int A_size[2] = {1024, 1024};
constexpr int B_size[2] = {1024, 1024};
#else
// Tiny matrix
constexpr int N = 2048;
constexpr int A_size[2] = {N, N};
constexpr int B_size[2] = {N, N};
#endif

float A[A_size[0] * A_size[1]];
float B[B_size[0] * B_size[1]];
float C[A_size[0] * B_size[1]];

int main() {
  std::mt19937 gen(/* seed */ 0);
  std::normal_distribution<> d{0, 1};
  for (int i = 0; i < A_size[0] * A_size[1]; i++) {
    A[i] = d(gen);
  }
  for (int i = 0; i < B_size[0] * B_size[1]; i++) {
    B[i] = d(gen);
  }
  if (A_size[1] != B_size[0]) {
    std::cerr << "size mismatch\n";
    return 1;
  }
  int C_size[2] = {A_size[0], B_size[1]};
#define MM_IMPL_MKL 0
#define MM_IMPL_NAIVE 1

#define MM_IMPL MM_IMPL_MKL

#if MM_IMPL == MM_IMPL_MKL
  // NB: sgemm_ follows FORTRAN convention, which makes all of the input
  // arguments kind of nutty.  Recall the meaning of sgemm parameters:
  //
  //                                      FORTRAN       TORCH
  //    transa == 'n' ==> input matrix is normal      / transposed
  //    transa == 't' ==> input matrix is transposed  / normal
  //
  //    lda/ldb/ldc   ==> stride (given the layout)
  //
  // Ordinarily, sgemm wants to compute matrix multiply of A and B
  // assuming that the underlying layout is FORTRAN.
  //
  // Given an (n, m) matrix in TORCH layout (stride=m), this matrix
  // represents an (m, n) matrix in FORTRAN layout (stride=m).
  // (Notice the stride never changes; the order of sizes is what
  // changes!)
  //
  // So we have:
  //
  //  Inputs:   TORCH     FORTRAN
  //
  //    A       (n, m) -> (m, n)
  //    B       (m, k) -> (k, m)
  //    C       (n, k) <- (k, n)
  //
  // If we do a regular matrix multiply in Fortran space, but
  // with the arguments swapped, we get what we want in Torch space.
  //
  char transa = 'n';
  char transb = 'n';
  // Swapped!
  int m = C_size[1];
  int n = C_size[0];
  int k = B_size[0];
  float alpha = 1;
  float beta = 0;
  int lda = A_size[1];
  int ldb = B_size[1];
  int ldc = C_size[1];
  sgemm_(&transa, &transb, &m, &n, &k, &alpha, B, &ldb, A, &lda, &beta, C, &ldc);
#elif MM_IMPL == MM_IMPL_NAIVE
  for (int i = 0; i < C_size[0] * C_size[1]; i++) {
    C[i] = 0;
  }
  for (int i = 0; i < A_size[0]; i++) {
    for (int j = 0; j < A_size[1]; j++) {
      for (int k = 0; k < B_size[1]; k++) {
        C[i*C_size[1] + k] += A[i*A_size[1] + j] + B[j*B_size[1] + k];
      }
    }
  }
#else
#error "invalid MM_IMPL"
#endif
  return 0;
}
