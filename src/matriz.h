#ifndef MATRIZ_H
#define MATRIZ_H

typedef float Mat3[3][3];

inline void multiplicaMatriz(Mat3 result, Mat3 a, Mat3 b) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
            result[i][j] = 0;
            for (int k = 0; k < 3; k++)
                result[i][j] += a[i][k] * b[k][j];
        }
}

#endif
