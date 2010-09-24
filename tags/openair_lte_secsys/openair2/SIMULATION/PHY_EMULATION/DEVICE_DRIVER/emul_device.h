
#ifndef EMUL_DEVICE_H
#define EMUL_DEVICE_H


#define EMUL_IOC_MAGIC         'm'

#define EMUL_DUMP_CONFIG                 _IOR(EMUL_IOC_MAGIC,1,int)
#define EMUL_GET_TOPOLOGY                _IOR(EMUL_IOC_MAGIC,2,int)
#define EMUL_START                       _IOR(EMUL_IOC_MAGIC,3,int)
#define EMUL_STOP                        _IOR(EMUL_IOC_MAGIC,4,int)
#define EMUL_MAXNR         5



#endif /* EMUL_DEVICE_H */
