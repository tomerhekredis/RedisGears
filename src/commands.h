/*
 * commands.h
 *
 *  Created on: 10 Nov 2018
 *      Author: root
 */

#ifndef SRC_COMMANDS_H_
#define SRC_COMMANDS_H_


#include "redisgears.h"

void Command_ReturnResults(ExecutionPlan* gearsCtx, RedisModuleCtx *ctx);
void Command_ReturnResult(RedisModuleCtx* rctx, Record* record);
void Command_ReturnErrors(ExecutionPlan* gearsCtx, RedisModuleCtx *ctx);
void Command_ReturnResultsAndErrors(ExecutionPlan* gearsCtx, RedisModuleCtx *ctx);
int Command_FlushRegistrationsStats(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int Command_AbortExecution(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int Command_DropExecution(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int Command_GetResults(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int Command_GetResultsBlocking(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int Command_ExecutionGet(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int Command_Register(SessionRegistrationCtx* srctx, SessionRegistrationCtx_OnDone onDone, void *pd, char **err);

int Command_Init();

#endif /* SRC_COMMANDS_H_ */
