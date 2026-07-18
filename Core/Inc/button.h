#ifndef BUTTON_H
#define BUTTON_H

#include "main.h"
#include <stdint.h>

/* ---------------- Matrix geometry ---------------- */
#define MATRIX_ROWS   7
#define MATRIX_COLS   8
#define MATRIX_BUTTON_COUNT   (MATRIX_ROWS * MATRIX_COLS)

/* Number of consecutive matching scans required before a state change
 * is accepted (simple debounce). */
#define MATRIX_DEBOUNCE_COUNT 3


/* Row pins: driven low one at a time, otherwise held high (idle). */
typedef struct
{
    GPIO_TypeDef *port;
    uint16_t      pin;
} MatrixPin_t;

/* Fill these tables in button_matrix.c to match your wiring. */
extern const MatrixPin_t g_matrix_rows[MATRIX_ROWS];
extern const MatrixPin_t g_matrix_cols[MATRIX_COLS];

/**
 * @brief Scan the full matrix once, apply debounce, and return the
 *        current stable button bitmap.
 * @return Debounced bitmap, bit n set = button n pressed.
 */
uint8_t* ButtonMatrix_Scan(void);


#endif // BUTTON_H