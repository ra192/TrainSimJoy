#include "button.h"
#include "main.h"
#include <stdbool.h>


const MatrixPin_t g_matrix_rows[MATRIX_ROWS] =
{
    { BTN_R1_GPIO_Port,  BTN_R1_Pin},
    { BTN_R2_GPIO_Port, BTN_R2_Pin },
    { BTN_R3_GPIO_Port, BTN_R3_Pin },
    { BTN_R4_GPIO_Port, BTN_R4_Pin },
    { BTN_R5_GPIO_Port, BTN_R5_Pin },
    { BTN_R6_GPIO_Port, BTN_R6_Pin },
    { BTN_R7_GPIO_Port, BTN_R7_Pin },
};
 
const MatrixPin_t g_matrix_cols[MATRIX_COLS] =
{
    { BTN_C1_GPIO_Port,  BTN_C1_Pin },
    { BTN_C2_GPIO_Port, BTN_C2_Pin },
    { BTN_C3_GPIO_Port, BTN_C3_Pin },
    { BTN_C4_GPIO_Port, BTN_C4_Pin },
    { BTN_C5_GPIO_Port, BTN_C5_Pin },
    { BTN_C6_GPIO_Port, BTN_C6_Pin },
    { BTN_C7_GPIO_Port, BTN_C7_Pin },
    { BTN_C8_GPIO_Port, BTN_C8_Pin },
};

static uint8_t s_stable_state[MATRIX_ROWS]    = {0};
static uint8_t s_candidate_state[MATRIX_ROWS] = {0};
static uint8_t        s_agree_count[MATRIX_BUTTON_COUNT] = {0};

uint8_t* ButtonMatrix_Scan(void)
{
    for (int r = 0; r < MATRIX_ROWS; r++)
    {
        /* Drive only this row low, all others stay high. */
        HAL_GPIO_WritePin(g_matrix_rows[r].port, g_matrix_rows[r].pin, GPIO_PIN_RESET);
        // HAL_Delay(1); /* wait for signals to settle */

        for (int c = 0; c < MATRIX_COLS; c++)
        {
            GPIO_PinState pin_state = !HAL_GPIO_ReadPin(g_matrix_cols[c].port, g_matrix_cols[c].pin);
            bool candidate_bit = (s_candidate_state[r] >>c) & 0x1;
            int button_index = r * MATRIX_COLS + c;
            
            if(pin_state == candidate_bit)
            {
                if (s_agree_count[button_index] < MATRIX_DEBOUNCE_COUNT)
                {
                    s_agree_count[button_index]++;
                }

                if (s_agree_count[button_index] >= MATRIX_DEBOUNCE_COUNT)
                {
                    s_stable_state[r] &= ~(1UL << c);
                    s_stable_state[r] |= (pin_state << c);
                }
            }
            else
            {
                /* Raw reading disagrees with the current candidate: start
                 * tracking the new value fresh. */
                s_candidate_state[r] &= ~(1UL << c);
                s_candidate_state[r] |= (pin_state << c);
                s_agree_count[button_index] = 1;
            }
            
        }
 
        /* Release the row before moving to the next one. */
        HAL_GPIO_WritePin(g_matrix_rows[r].port, g_matrix_rows[r].pin, GPIO_PIN_SET);
    }
 
    return s_stable_state;
}