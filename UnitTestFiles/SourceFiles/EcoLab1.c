/*
 * <кодировка символов>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </кодировка символов>
 *
 * <сводка>
 *   EcoLab1
 * </сводка>
 *
 * <описание>
 *   Данный исходный файл является точки входа
 * </описание>
 *
 * <автор>
 *   Copyright (c) 2018 Vladimir Bashev. All rights reserved.
 * </автор>
 *
 */


/* Eco OS */
#include "IEcoSystem1.h"
#include "IdEcoMemoryManager1.h"
#include "IdEcoInterfaceBus1.h"
#include "IdEcoFileSystemManagement1.h"
#include "IdEcoLab1.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 *
 * <сводка>
 *   Функция сравнения для qsort
 * </сводка>
 *
 */
int __cdecl compare(const void* a, const void* b) {
    int32_t valueA = 0;
    int32_t valueB = 0;
    
    valueA = *(int32_t*)a;
    valueB = *(int32_t*)b;
    
    return (valueA - valueB);
}

/*
 *
 * <сводка>
 *   Функция проверки отсортированности массива
 * </сводка>
 *
 */
int is_sorted(int32_t* arr, uint32_t size) {
    uint32_t i = 0;
    
    for (i = 1; i < size; i++) {
        if (arr[i] < arr[i-1]) {
            return 0;
        }
    }
    return 1;
}

/*
 *
 * <сводка>
 *   Функция заполнения массива случайными числами
 * </сводка>
 *
 */
void fill_random(int32_t* arr, uint32_t size) {
    uint32_t i = 0;
    
    for (i = 0; i < size; i++) {
        arr[i] = rand() % 1000000;
    }
}

/*
 *
 * <сводка>
 *   Функция копирования массива
 * </сводка>
 *
 */
void copy_array(int32_t* dest, int32_t* src, uint32_t size) {
    uint32_t i = 0;
    
    for (i = 0; i < size; i++) {
        dest[i] = src[i];
    }
}

/*
 *
 * <сводка>
 *   Функция EcoMain
 * </сводка>
 *
 * <описание>
 *   Функция EcoMain - точка входа
 * </описание>
 *
 */
int16_t EcoMain(IEcoUnknown* pIUnk) {
    int16_t result = -1;
    /* Указатель на системный интерфейс */
    IEcoSystem1* pISys = 0;
    /* Указатель на интерфейс работы с системной интерфейсной шиной */
    IEcoInterfaceBus1* pIBus = 0;
    /* Указатель на интерфейс работы с памятью */
    IEcoMemoryAllocator1* pIMem = 0;
    /* Указатель на тестируемый интерфейс */
    IEcoLab1* pIEcoLab1 = 0;

    /* Размеры массивов для тестирования */
    uint32_t sizes[5] = {10, 100, 1000, 10000, 100000};
    int num_sizes = 0;
    int i = 0;
    uint32_t size = 0;
    int32_t* arr1 = 0;
    int32_t* arr2 = 0;
    clock_t start = 0;
    clock_t end = 0;
    double our_time = 0.0;
    double qsort_time = 0.0;

    /* Инициализация генератора случайных чисел */
    srand((unsigned int)time(NULL));

    num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    /* Проверка и создание системного интрефейса */
    if (pISys == 0) {
        result = pIUnk->pVTbl->QueryInterface(pIUnk, &GID_IEcoSystem, (void **)&pISys);
        if (result != 0 && pISys == 0) {
        /* Освобождение системного интерфейса в случае ошибки */
            goto Release;
        }
    }

    /* Получение интерфейса для работы с интерфейсной шиной */
    result = pISys->pVTbl->QueryInterface(pISys, &IID_IEcoInterfaceBus1, (void **)&pIBus);
    if (result != 0 || pIBus == 0) {
        /* Освобождение в случае ошибки */
        goto Release;
    }
#ifdef ECO_LIB
    /* Регистрация статического компонента для работы со списком */
    result = pIBus->pVTbl->RegisterComponent(pIBus, &CID_EcoLab1, (IEcoUnknown*)GetIEcoComponentFactoryPtr_1F5DF16EE1BF43B999A434ED38FE8F3A);
    if (result != 0 ) {
        /* Освобождение в случае ошибки */
        goto Release;
    }
#endif
    /* Получение интерфейса управления памятью */
    result = pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoMemoryManager1, 0, &IID_IEcoMemoryAllocator1, (void**) &pIMem);

    /* Проверка */
    if (result != 0 || pIMem == 0) {
        /* Освобождение системного интерфейса в случае ошибки */
        goto Release;
    }

    /* Получение тестируемого интерфейса */
    result = pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoLab1, 0, &IID_IEcoLab1, (void**) &pIEcoLab1);
    if (result != 0 || pIEcoLab1 == 0) {
        /* Освобождение интерфейсов в случае ошибки */
        goto Release;
    }

    printf("Testing Insertion Sort vs qsort performance\n");
    printf("===========================================\n");

    /* Тестирование для каждого размера */
    for (i = 0; i < num_sizes; i++) {
        size = sizes[i];
        printf("\nTesting with array size: %u\n", size);

        /* Выделение памяти для массивов */
        arr1 = (int32_t*)pIMem->pVTbl->Alloc(pIMem, size * sizeof(int32_t));
        arr2 = (int32_t*)pIMem->pVTbl->Alloc(pIMem, size * sizeof(int32_t));

        if (arr1 == 0 || arr2 == 0) {
            printf("Memory allocation failed for size %u\n", size);
            if (arr1 != 0) {
                pIMem->pVTbl->Free(pIMem, arr1);
            }
            if (arr2 != 0) {
                pIMem->pVTbl->Free(pIMem, arr2);
            }
            continue;
        }

        /* Заполнение массива случайными числами */
        fill_random(arr1, size);
        copy_array(arr2, arr1, size);

        /* Тестирование нашей сортировки */
        start = clock();
        result = pIEcoLab1->pVTbl->MyFunction(pIEcoLab1, arr1, size);
        end = clock();
        our_time = ((double)(end - start)) / CLOCKS_PER_SEC;

        if (result == 0 && is_sorted(arr1, size)) {
            printf("Our Insertion Sort: %.6f seconds - OK\n", our_time);
        } else {
            printf("Our Insertion Sort: FAILED\n");
        }

        /* Тестирование qsort */
        start = clock();
        qsort(arr2, size, sizeof(int32_t), compare);
        end = clock();
        qsort_time = ((double)(end - start)) / CLOCKS_PER_SEC;

        if (is_sorted(arr2, size)) {
            printf("Standard qsort:      %.6f seconds - OK\n", qsort_time);
        } else {
            printf("Standard qsort:      FAILED\n");
        }

        printf("Speed ratio (qsort/our): %.2fx\n", qsort_time!=0 ? our_time / qsort_time : 0);

        /* Освобождение памяти */
        pIMem->pVTbl->Free(pIMem, arr1);
        pIMem->pVTbl->Free(pIMem, arr2);
        
        /* Сброс указателей */
        arr1 = 0;
        arr2 = 0;
    }

    printf("\nTesting completed. Press any key to exit...\n");
    getchar();

Release:

    /* Освобождение интерфейса для работы с интерфейсной шиной */
    if (pIBus != 0) {
        pIBus->pVTbl->Release(pIBus);
    }

    /* Освобождение интерфейса работы с памятью */
    if (pIMem != 0) {
        pIMem->pVTbl->Release(pIMem);
    }

    /* Освобождение тестируемого интерфейса */
    if (pIEcoLab1 != 0) {
        pIEcoLab1->pVTbl->Release(pIEcoLab1);
    }

    /* Освобождение системного интерфейса */
    if (pISys != 0) {
        pISys->pVTbl->Release(pISys);
    }

    return result;
}