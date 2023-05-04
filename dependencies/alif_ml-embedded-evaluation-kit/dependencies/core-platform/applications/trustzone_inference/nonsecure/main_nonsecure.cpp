/*
 * Copyright (c) 2019-2021 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* to be able to print from nonsecure world since uart is mapped
 * to secure world. */
#include "../common/secure_entries.hpp"

/* Disable semihosting */
__asm(".global __use_no_semihosting\n");

/* Target specific CMSIS Device header is included by compiler,
 * see cmsis.cmake in core_software */

using namespace std;

extern "C" void SystemInit() {
    /* secure world is doing the neccessary setup, since it has
     * the privilege to do so, and non-secure doesn't
     */
    nonsecure_print("skipping system init from non-secure world");
}

/*
 * Retargeting stubs for the non-secure world
 */
extern "C" {

void _sys_exit(int code) {
    (void)code;
    nonsecure_print("Returning to secure world!");
    /*
     * Make sure we return to secure world when exit by
     * setting LR to FNC_RETURN and branching to tell CPU
     * we want to leave the non-secure world
     */
    asm("LDR r14, =0xFEFFFFFE\n");
    asm("BX lr\n");
}

void _ttywrch(int ch) {
    (void)ch;
}

char *_sys_command_string(char *cmd, int len) {
    (void)len;

    return cmd;
}

} // extern "C"

int result = -1;

int return_nonsecure_result(void) {
    return result;
}

int main() {
    nonsecure_print("Non-secure main starting up.");

    set_result_function(return_nonsecure_result);

    /* Execute inference in secure world with data not accessible from
     * non-secure world
     */
    nonsecure_print("Starting secure inference.");
    int inference_failed = run_secure_inference();

    nonsecure_print("Inference returned: ");
    nonsecure_print(inference_failed ? "failed" : "success");

    result = inference_failed ? -1 : 0;

    return 0;
}
