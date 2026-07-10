#include "button.h"
#include "main.h"
#include <stdbool.h>


const MatrixPin_t g_matrix_rows[MATRIX_ROWS] =
{
    { BTN_R1_GPIO_Port,  BTN_R1_Pin},
    { BTN_R2_GPIO_Port, BTN_R2_Pin },
    { BTN_R3_GPIO_Port, BTN_R3_Pin },
    { BTN_R4_GPIO_Port, BTN_R4_Pin },
};
 
const MatrixPin_t g_matrix_cols[MATRIX_COLS] =
{
    { BTN_C1_GPIO_Port,  BTN_C1_Pin },
    { BTN_C2_GPIO_Port, BTN_C2_Pin },
    { BTN_C3_GPIO_Port, BTN_C3_Pin },
    { BTN_C4_GPIO_Port, BTN_C4_Pin },
};

static ButtonBitmap_t s_stable_state    = 0;
static ButtonBitmap_t s_candidate_state = 0;
static uint8_t        s_agree_count[MATRIX_BUTTON_COUNT] = {0};


ButtonBitmap_t ButtonMatrix_Scan(void)
{
    ButtonBitmap_t raw_state = 0;
 
    for (int r = 0; r < MATRIX_ROWS; r++)
    {
        /* Drive only this row low, all others stay high. */
        HAL_GPIO_WritePin(g_matrix_rows[r].port, g_matrix_rows[r].pin, GPIO_PIN_RESET);
        // HAL_Delay(1); /* wait for signals to settle */

        for (int c = 0; c < MATRIX_COLS; c++)
        {
            GPIO_PinState pin_state = HAL_GPIO_ReadPin(g_matrix_cols[c].port, g_matrix_cols[c].pin);
            if (pin_state == GPIO_PIN_RESET) /* pulled low through the switch = pressed */
            {
                int idx = (r * MATRIX_COLS) + c;
                raw_state |= (1UL << idx);
            }
        }
 
        /* Release the row before moving to the next one. */
        HAL_GPIO_WritePin(g_matrix_rows[r].port, g_matrix_rows[r].pin, GPIO_PIN_SET);
    }
 
        /* --- Simple per-button debounce --- */
    for (int i = 0; i < MATRIX_BUTTON_COUNT; i++)
    {
        bool raw_bit       = (raw_state       >> i) & 0x1;
        bool candidate_bit = (s_candidate_state >> i) & 0x1;
 
        if (raw_bit == candidate_bit)
        {
            if (s_agree_count[i] < MATRIX_DEBOUNCE_COUNT)
            {
                s_agree_count[i]++;
            }
        }
        else
        {
            /* Raw reading disagrees with the current candidate: start
             * tracking the new value fresh. */
            s_candidate_state &= ~(1UL << i);
            s_candidate_state |= ((ButtonBitmap_t)raw_bit << i);
            s_agree_count[i] = 1;
        }
 
        if (s_agree_count[i] >= MATRIX_DEBOUNCE_COUNT)
        {
            s_stable_state &= ~(1UL << i);
            s_stable_state |= ((ButtonBitmap_t)candidate_bit << i);
        }
    }
 
    return s_stable_state;
}