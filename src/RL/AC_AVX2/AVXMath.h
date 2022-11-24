#ifndef AVX_MATH_H
#define AVX_MATH_H


static inline float _mm256_reduce_add_ps(__m256 x) {
    /* ( x3+x7, x2+x6, x1+x5, x0+x4 ) */
    const __m128 x128 = _mm_add_ps(_mm256_extractf128_ps(x, 1), _mm256_castps256_ps128(x));
    /* ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 ) */
    const __m128 x64 = _mm_add_ps(x128, _mm_movehl_ps(x128, x128));
    /* ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 ) */
    const __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
    /* Conversion to float is a no-op on x86-64 */
    return _mm_cvtss_f32(x32);
}


// credit goes to ???

static inline __m256 _mm256_exp_ps(__m256 _x) {
    __m256 c1 = _mm256_set1_ps(0.007972914726F);
    __m256 c2 = _mm256_set1_ps(0.1385283768F);
    __m256 c3 = _mm256_set1_ps(2.885390043F);
    __m256 c4 = _mm256_set1_ps(1.442695022F);
    __m256 x = _mm256_mul_ps(_x, c4); //convert to 2^(x)
    __m256 intPartf = _mm256_round_ps(x, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
    x = _mm256_sub_ps(x, intPartf);
    __m256 xx = _mm256_mul_ps(x, x);
    __m256 a = _mm256_add_ps(x, _mm256_mul_ps(c1, _mm256_mul_ps(xx, x))); //can be improved with FMA
    __m256 b = _mm256_add_ps(c3, _mm256_mul_ps(c2, xx));
    __m256 res = _mm256_div_ps(_mm256_add_ps(b, a), _mm256_sub_ps(b, a));
    __m256i intPart = _mm256_cvtps_epi32(intPartf); //res = 2^intPart. Can be improved with AVX2!
    __m128i ii0 = _mm_slli_epi32(_mm256_castsi256_si128(intPart), 23);
    __m128i ii1 = _mm_slli_epi32(_mm256_extractf128_si256(intPart, 1), 23);
    __m128i res_0 = _mm_add_epi32(ii0, _mm256_castsi256_si128(_mm256_castps_si256(res)));
    __m128i res_1 = _mm_add_epi32(ii1, _mm256_extractf128_si256(_mm256_castps_si256(res), 1));
    return _mm256_insertf128_ps(_mm256_castsi256_ps(_mm256_castsi128_si256(res_0)), _mm_castsi128_ps(res_1), 1);
}

static inline float fast_exp(float x) {
    const float c1 = 0.007972914726F;
    const float c2 = 0.1385283768F;
    const float c3 = 2.885390043F;
    const float c4 = 1.442695022F;
    x *= c4; //convert to 2^(x)
    int intPart = (int)x;
    x -= intPart;
    float xx = x * x;
    float a = x + c1 * xx * x;
    float b = c3 + c2 * xx;
    float res = (b + a) / (b - a);
    reinterpret_cast<int &>(res) += intPart << 23; // res *= 2^(intPart)
    return res;
}

static inline float l2_norm(float** parameters, int dim_x, int dim_y) {
    __m256 results = _mm256_setzero_ps();
    for (int i = 0; i < dim_x; i++) {
        for (int j = 0; j < dim_y / 8; j++) {
            __m256 parametersVector = _mm256_load_ps(&parameters[i][j * 8]);
            results = _mm256_add_ps(results, _mm256_mul_ps(parametersVector, parametersVector));
        }
    }
    return std::sqrt(_mm256_reduce_add_ps(results));
}


static inline float l2_norm(float* parameters, int dim) {
    __m256 results = _mm256_setzero_ps();
    for (int j = 0; j < dim / 8; j++) {
        __m256 parametersVector = _mm256_load_ps(&parameters[j * 8]);
        results = _mm256_add_ps(results, _mm256_mul_ps(parametersVector, parametersVector));
    }
    return std::sqrt(_mm256_reduce_add_ps(results));
}
#endif //DLB_MATH_H
