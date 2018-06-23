#ifndef __MI_SYS_DEBUG_H__
#define __MI_SYS_DEBUG_H__


#define __MI_SYS_ENABLE_DEBUG 1


#define __MI_SYS_MAGIC__(spell)             ( (((spell)[sizeof(spell)*8/9] << 24) | \
                                      ((spell)[sizeof(spell)*7/9] << 16) | \
                                      ((spell)[sizeof(spell)*6/9] <<  8) | \
                                      ((spell)[sizeof(spell)*5/9]      )) ^  \
                                     (((spell)[sizeof(spell)*4/9] << 24) | \
                                      ((spell)[sizeof(spell)*3/9] << 16) | \
                                      ((spell)[sizeof(spell)*2/9] <<  8) | \
                                      ((spell)[sizeof(spell)*1/9]      )) )

#if __MI_SYS_ENABLE_DEBUG



#define MI_SYS_ASSERT(exp)     do {BUG_ON(!(exp));}while(0)
#define MI_SYS_BUG()     do{BUG();}while(0)


/*
 * Magic Assertions & Utilities
 */


#define __MI_SYS_MAGIC_PRINT__(m)           do {                                              \
                                        printk(KERN_ERR "Magic '%s' = '0x%08x'\n", #m, D_MAGIC(#m) );       \
                                   } while (0)


#define MI_SYS_MAGIC_CHECK(o,m)         \
     do{  \
           MI_SYS_ASSERT((o) != NULL);   \
          if((o)->magic != D_MAGIC(#m))  \
          {   \
               __MI_SYS_MAGIC_PRINT__(m);   \
               printk(KERN_ERR "     current magic is '0x%08x\n", (o)->magic));   \
               MI_SYS_BUG();   \
          }  \
     }while(0)

#define MI_SYS_MAGIC_SET(o,m)           do {                                              \
                                        MI_SYS_ASSERT( (o) != NULL );                     \
                                        (o)->magic = __MI_SYS_MAGIC__(#m);                    \
                                   } while (0)

#define MI_SYS_MAGIC_CLEAR(o)           do {                                              \
                                        MI_SYS_ASSERT( (o) != NULL );                     \
                                        MI_SYS_ASSERT( (o)->magic != 0 );                 \
                                        (o)->magic = 0;                              \
                                   } while (0)
#else

#define MI_SYS_ASSERT(exp)     do {}while(0)
#define MI_SYS_BUG()     do{ }while(0)

#define MI_SYS_MAGIC_CHECK(o,m)  do{ }while(0)

#define MI_SYS_MAGIC_SET(o,m)           do {                                              \
                                        (o)->magic = __MI_SYS_MAGIC__(#m);                    \
                                   } while (0)
#define MI_SYS_MAGIC_CLEAR(o)     do{ }while(0)
#endif


#define MI_SYS_INFO(...)      do{printk(__VA_ARGS__);}while(0)

#define MI_SYS_WARN(...)     do{printk(KERN_ERR __VA_ARGS__);}while(0)

#define MI_SYS_ERROR(...)      do{printk(__VA_ARGS__);}while(0)


#endif