// Audit trail function prototype file
// File:  audit.hpp
// Created 8/2/95 by Jason Jones
#ifndef AUDIT_HPP
#define AUDIT_HPP

#include "basetask.hpp"

#define AUDITLENGTH 1000

// Function Prototypes
void WriteAuditTrail(char *aFileName);
void WriteAuditTrailNum(char *aFileName);
void AuditTrail(BaseTask *Task, int aFromState, int aToState);
void InitAuditTrail(void);

#endif // AUDIT_HPP
