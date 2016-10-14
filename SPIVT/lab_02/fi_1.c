/*
 * Программа, сожержит функции кодирования и декодирования данных
 * Кодировка выполняется с помощью функции фи1, описанной в методическом пособии ЛР-1.pdf
 * 
 * Этапы кодирования:
 * 1. Порционное считывание данных (по 1-ому байту) из кодируемого файла.
 * 2. Определение "бинарной длины" (length) считавшейся порции данных
 *    (количество значимых бит в двоичном представлении).
 * 3. Кодирование порции данных согласно функции фи1:
 *  3.1. Кодирование "бинарной длины" с помощью функции фи0 (фи0(length)).
 *  3.2. Запись результата, полученного на шаге 3.1. в конец выходного файла.
 *  3.3. Запись 'length - 1' младших бит считанной порции данных в конец выходного файла.
 * 4. Повтор шагов 1-3, до тех пор, пока не закончится содержимое кодируемого файла.
 *
 * Этапы декодирования:
 *  
 *  Порционное считывание данных
 *  (по 1-ому байту) из входного файла
 *  происходит по мере необходимости!
 *
 * 1. Определение "бинарной длины" (length) закодированной порции данных
 *    (подсчёт подрядидущих нулевых бит до первого ненулевого бита).
 * 2. Сичтывание 'length' следующих бит (включая первый ненулевой бит с шага 1).
 * 3. Декодирование полученных данных согласно функции фи1.
 * 4. Запись результата, полученного на шаге 3, в конец выходного файла.
 * 5. Повтор шагов 1-4, до тех пор, пока не закончится содержимое декодируемого файла.
 */
#include <stdio.h>


/*
 * Функция определения "бинарной длины" 
 * одного символа размером 1 байт
 */
short int bival_length (char *val)
{
    /* Максимальная длина может равняться 8 битам */
    short int length = 8;

    /* Поиск первого ненулевого бита (слева направа) */
    while ( !(*val >> (length - 1)) ) {
        if ( !(--length) ) {
            break;
        }
    }

    return length;
}



/*
 * Функция кодирования данных c помощью фи1
 * fin  - входной поток
 * fout - выходной поток
 */
void fi_1_coding (FILE *fin, FILE *fout)
{
    char c_8;     // Буфер для считывания порции данных (8 бит)
    char c = 0x0; // Буфер для записи закодированной последовательности (8 бит)
    int  i = 0;   // Счётчик записанных нулевых бит Х 8

    short int c_16;    // Буфер для преобразования символа в число (16 бит)
    short int length;  // "Бинарная длина"

    /* Текущая позиции обрабатываемого бита в буфере 'c' */
    short int cur_bit_pos = 7;

    /* Порционное считывание данных (по 1-ому байту) */
    while ( fread (&c_8, 1, sizeof (char), fin) ) {
        /* Определение длины */
        length = c_16 = bival_length (&c_8);

        /* Обработка оставшегося места в буфере 'c' */
        if (c != 0x0 || cur_bit_pos < 7) {
            if (c_16 - (cur_bit_pos + 1) >= 0) {
                c_16 -= cur_bit_pos + 1;
                fwrite (&c, 1, sizeof (char), fout);
                c = 0x0;
            } else {
                c_16 = 7 - (cur_bit_pos - c_16);
            }
        }

        /* Запись в конец выходного файла нулевых бит Х 8 */
        for (i = 0; i < c_16 / 8; ++i) {
            fwrite (&c, 1, sizeof (char), fout);
        }

        /* Добавление единичного бита в закодированную последовательность */
        c |= 1 << (7 - (c_16 - i * 8));
        cur_bit_pos = 7 - (c_16 - i * 8) - 1;

        --length;   // Отбрасывание ведущего единичного бита в двоичном виде

       /*
        * Добавление в буфер 'c' кодируемого символа 
        * без ведущего единичного бита в двоичном представлении
        */
        while ( (length > 0) && (cur_bit_pos >= 0) ) {
            if ( (c_8 >> (length - 1)) & 1 ) {
                c |= 1 << cur_bit_pos;
            } 
            --cur_bit_pos;
            --length;
        }
        
        /* 
         * Если буфер 'c' заполнился, 
         * сбросить его содержимое в конец выходного файла
         */
        if (length > 0) {
            fwrite (&c, 1, sizeof (char), fout);
            c = 0x0;
            cur_bit_pos = 7;
        }

        /* Обработка оставшейся части кодируемого символа */
        while (length) {
            if ( (c_8 >> (length - 1)) & 1 ) {
                c |= 1 << cur_bit_pos;
            } 
            --cur_bit_pos;
            --length;
        } 

        /* 
         * Если буфер 'c' заполнился, 
         * сбросить его содержимое в конец выходного файла
         */
        if (cur_bit_pos == -1) {
            fwrite (&c, 1, sizeof (char), fout);
            c = 0x0;
            cur_bit_pos = 7;
        }
    }
    
   /* 
    * Если буфер 'c' не пуст, 
    * сбросить его содержимое в конец выходного файла
    */
    if (cur_bit_pos != 7) {
        fwrite (&c, 1, sizeof (char), fout);
    }
}




/*
 * Функция декодирования данных с помощью фи1
 * fin  - входной поток
 * fout - выходной поток
 */
void fi_1_decoding (FILE *fin, FILE *fout)
{
    char c_8;     // Буфер для считывания порции данных (8 бит)
    char c = 0x0; // Буфер для записи декодированного символа в выходной файл

    short int length = 0; // "Бинарная длина"

    /* Порционное считывание закодированных данных (по 1 байту) */
    while ( fread (&c_8, 1, sizeof (char), fin) ) {
        /* Обработка порции данных */
        for (int i = 7; i >= 0; --i) {
            /* Если встретился ненулевой бит */
            if ( (c_8 >> i) & 1 ) {
                /* Если закодирован ноль */
                if (!length) {
                    c = 0x1;
                /* Декодировние символа*/
                } else {
                    c |= 1 << (--length);
                }
            } else {
                if (c) {
                    --length;  // Декодирование символа
                } else {
                    ++length;  // Подсчёт "бинарной длины"
                }
            }
            
            /* Запись декодированного символа в конец выходного файла */
            if (!length) {
                fwrite (&c, 1, sizeof (char), fout);
                c = 0x0;  // Сброс буфера 'c'
            }
        }
    }
}