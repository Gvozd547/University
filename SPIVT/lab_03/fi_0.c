/*
 * Программа, сожержит функции кодирования и декодирования данных
 * Кодировка выполняется с помощью функции фи0, описанной в методическом пособии ЛР-1.pdf
 * 
 * Этапы кодирования:
 * 1. Порционное считывание данных (по 1-ому байту) из кодируемого файла.
 * 2. Преобразование символа в целочисленный вид (в число N).
 * 3. Кодирование порции данных согласно функции фи0:
 *  3.1. Запись N нулевых бит в конец выходного файла.
 *  3.2. Запись единичного бита в конец выходного файла.
 * 4. Повтор шагов 1-3, до тех пор, пока не закончится содержимое кодируемого файла.
 *
 * Этапы декодирования:
 *  
 *  Порционное считывание данных
 *  (по 1-ому байту) из входного файла
 *  происходит по мере необходимости!
 *
 * 1. Подсчёт подрядидущих нулевых бит до первого ненулевого бита.
 * 2. Декодирование полученных данных согласно функции фи0
 *    (результат, полученный на шаге 1, является закодированным числом/символом).
 * 3. Запись результата, полученного на шаге 2, в конец выходного файла.
 * 4. Повтор шагов 1-3, до тех пор, пока не закончится содержимое декодируемого файла.
 */
#include <stdio.h>


/*
 * Функция кодирования данных с помощью фи0
 * in_file  - входной файл
 * out_file - выходной файл
 */
void fi_0_coding (char *in_file, char *out_file)
{
    FILE *fin, *fout;

    fin  = fopen (in_file, "rb");
    fout = fopen (out_file, "wb");

    char c_8;     // Буфер для считывания порции данных (8 бит)
    char c = 0x0; // Буфер для записи закодированной последовательности (8 бит)
    int  i = 0;   // Счётчик записанных нулевых бит Х 8

    short int c_16;   // Буфер для преобразования символа в число (16 бит)
  
    /* Текущая позиции обрабатываемого бита в буфере 'c' */
    short int cur_bit_pos = 7;

    /* Порционное считывание данных (по 1-ому байту) */
    while ( fread (&c_8, 1, sizeof (char), fin) ) {
        /* Преобразование символа в число */
        c_16 = (short int) c_8;

        /* Обработка оставшегося места в буфере 'c' */
        if (c != 0x0) {
            if (c_16 - (cur_bit_pos + 1) >= 0) {
                c_16 -= cur_bit_pos + 1;
                fwrite (&c, 1, sizeof (char), fout);
                c = 0x0;
                cur_bit_pos = -2;
            } else {
                c |= 1 << (cur_bit_pos - c_16);
                cur_bit_pos -= c_16 + 1;
                continue;
            }
        }
        
        /* Запись в конец выходного файла нулевых бит Х 8 */
        for (i = 0; i < c_16 / 8; ++i) {
            fwrite (&c, 1, sizeof (char), fout);
        }

        /* Добавление единичного бита в закодированную последовательность */
        c |= 1 << (7 - (c_16 - i * 8));
        cur_bit_pos = 7 - (c_16 - i * 8) - 1;
    }
    
   /* 
    * Если буфер 'c' не пуст, 
    * сбросить его содержимое в конец выходного файла
    */
    if (cur_bit_pos != -2) {
        fwrite (&c, 1, sizeof (char), fout);
    }

    fclose (fin);
    fclose (fout);
}



/*
 * Функция декодирования данных с помощью фи0
 * in_file  - входной файл
 * out_file - выходной файл
 */
void fi_0_decoding (char *in_file, char *out_file)
{
    FILE *fin, *fout;

    fin  = fopen (in_file, "rb");
    fout = fopen (out_file, "wb");

    char c_8; // Буфер для считывания порции данных (8 бит)
    char c;   // Буфер для записи декодированного символа в выходной файл

    short int res = 0; // Декодированный символ

    /* Порционное считывание закодированных данных (по 1 байту) */
    while ( fread (&c_8, 1, sizeof (char), fin) ) {
        /* Обработка порции данных */
        for (int i = 7; i >= 0; --i) {
            /* Если встретился ненулевой бит */
            if ((c_8 >> i) & 1) {
                c = (char) res;
                /* Запись декодированного символа в конец выходного файла */
                fwrite (&c, 1, sizeof (char), fout);
                res = 0;
            } else {
                ++res;  // Подсчёт подрядидущих нулевых бит
            }
        }
    }

    fclose (fin);
    fclose (fout);
}
        
