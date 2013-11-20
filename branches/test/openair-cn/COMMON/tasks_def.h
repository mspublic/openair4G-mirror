// This task is mandatory and must always be placed in first position
TASK_DEF(TASK_TIMER,    TASK_PRIORITY_MED, 10)

// Other possible tasks in the process

/// GTPV1-U task
TASK_DEF(TASK_GTPV1_U,  TASK_PRIORITY_MED, 200)
/// FW_IP task
TASK_DEF(TASK_FW_IP,    TASK_PRIORITY_MED, 200)
/// MME Applicative task
TASK_DEF(TASK_MME_APP,  TASK_PRIORITY_MED, 200)
/// NAS task
TASK_DEF(TASK_NAS,      TASK_PRIORITY_MED, 200)
/// S1AP task
TASK_DEF(TASK_S11,      TASK_PRIORITY_MED, 200)
/// S1AP task
TASK_DEF(TASK_S1AP,     TASK_PRIORITY_MED, 200)
/// S6a task
TASK_DEF(TASK_S6A,      TASK_PRIORITY_MED, 200)
/// SCTP task
TASK_DEF(TASK_SCTP,     TASK_PRIORITY_MED, 200)
/// Serving and Proxy Gateway Application task
TASK_DEF(TASK_SPGW_APP, TASK_PRIORITY_MED, 200)
/// UDP task
TASK_DEF(TASK_UDP,      TASK_PRIORITY_MED, 200)
