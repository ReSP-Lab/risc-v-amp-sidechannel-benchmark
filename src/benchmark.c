/*
* This file generates benchmark binary based on provided compilation flags
*
* Copyright (C) 2024-2025 Cédrick Austa <cedrick.austa@ulb.be>
* Copyright (C) 2020 Shuwen Deng <shuwen.deng@yale.edu>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*
*/

#ifdef AMP_SETUP
    #include "amp_setup/benchmark_implem.h"
#else
    #include "linux_setup/benchmark_implem.h"
#endif /* ifdef AMP_SETUP */

#define RNG_SEED         1234567


#ifdef AMP_SETUP
__attribute__((section(".text.main")))
#endif /* ifdef AMP_SETUP */
int main(int argc, char *argv[])
{
    rng_srand(RNG_SEED);
#ifdef AMP_SETUP
    amp_main(argc, argv);
#else
    linux_main(argc, argv);
#endif /* ifdef AMP_SETUP */

    return EXIT_SUCCESS;
}
