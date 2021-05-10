/***************************************************************************
 *   Copyright (C) 2021 by Federico Amedeo Izzo IU2NUO,                    *
 *                         Niccolò Izzo IU2KIN,                            *
 *                         Frederik Saraci IU2NRO,                         *
 *                         Silvano Seva IU2KWO                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or  functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#ifndef MEMORY_PROFILING_H
#define MEMORY_PROFILING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \return stack size of the caller thread.
 */
unsigned int getStackSize();

/**
 * \return absolute free stack of current thread.
 * Absolute free stack is the minimum free stack since the thread was
 * created.
 */
unsigned int getAbsoluteFreeStack();

/**
 * \return current free stack of current thread.
 * Current free stack is the free stack at the moment when the this
 * function is called.
 */
unsigned int getCurrentFreeStack();

/**
 * \return heap size which is defined in the linker script The heap is
 * shared among all threads, therefore this function returns the same value
 * regardless which thread is called in.
 */
unsigned int getHeapSize();

/**
 * \return absolute (not current) free heap.
 * Absolute free heap is the minimum free heap since the program started.
 * The heap is shared among all threads, therefore this function returns
 * the same value regardless which thread is called in.
 */
unsigned int getAbsoluteFreeHeap();

/**
 * \return current free heap.
 * Current free heap is the free heap at the moment when the this
 * function is called.
 * The heap is shared among all threads, therefore this function returns
 * the same value regardless which thread is called in.
 */
unsigned int getCurrentFreeHeap();

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_PROFILING_H */
