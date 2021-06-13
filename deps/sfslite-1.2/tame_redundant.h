
# 1 "tame_redundant.Th"
// -*-c++-*-
#ifndef __TAME_REDUNDANT_H__
#define __TAME_REDUNDANT_H__

#include "async.h"
#include "tame.h"

/* MM: The redundant caller allows for placing a timer
 * on a tamed function call which will serve to place a 
 * timeout on that call, upon which we will spawn additional
 * tasks until one of the spawned tasks has returned.
 *  
 * This is useful if you have a multiple node system, where
 * many nodes are equivalent. You can dispatch a job on one, and
 * if it doesn't return fast enough, you can spawn on more and 
 * have them race. 
 */


template<typename ResultType, typename TaskType>
struct redundant_caller_t;


template<typename ResultType, typename TaskType>
struct redundant_caller_manager_t : public virtual refcount {
public:
    void add_task(size_t index, const TaskType& task) {
        callers[index] += task;
    }

    void call (size_t index,
            size_t delay,
            typename event<ResultType, int>::ptr res_ev, CLOSURE);

    redundant_caller_manager_t (size_t n) : count(0) {
        callers = New redundant_caller_t <ResultType, TaskType>[n];
    }
    void inc () {
        count++;
    }
    void dec () {
        if (count > 0) {
            count--;
        }
        if (!count) {
            delete [] callers;
            callers = NULL;
        }
    }
    virtual ~redundant_caller_manager_t () {
        if (callers) {
            delete [] callers;

        }
    }
private:
    size_t count;
    redundant_caller_t <ResultType, TaskType>* callers;
};


# 62 "tame_redundant.Th"
template< typename ResultType, typename TaskType > class redundant_caller_manager_t_ResultType_TaskType___call_9d10329b__closure_t : public closure_t { public:   redundant_caller_manager_t_ResultType_TaskType___call_9d10329b__closure_t (redundant_caller_manager_t< ResultType  ,  TaskType > *_self,  size_t index,  size_t delay,  typename event< ResultType  ,  int >::ptr res_ev) : closure_t ("tame_redundant.Th", "redundant_caller_manager_t< ResultType  ,  TaskType >::call", 66), _self (_self), _args (index, delay, res_ev), _stack (_self, index, delay, res_ev) {}   typedef void  (redundant_caller_manager_t< ResultType  ,  TaskType >::*method_type_t) ( size_t ,  size_t ,  typename event< ResultType  ,  int >::ptr , ptr<closure_t>);   void set_method_pointer (method_type_t m) { _method = m; }   void reenter ()   {     ((*_self).*_method)  (_args.index, _args.delay, _args.res_ev, mkref (this));   } void v_reenter () { reenter (); }   struct args_t {     args_t ( size_t index,  size_t delay,  typename event< ResultType  ,  int >::ptr res_ev) : index (index), delay (delay), res_ev (res_ev) {}      size_t index;      size_t delay;      typename event< ResultType  ,  int >::ptr res_ev;   };   redundant_caller_manager_t< ResultType  ,  TaskType > *_self;   args_t _args;   struct stack_t {     stack_t (redundant_caller_manager_t< ResultType  ,  TaskType > *_self,  size_t index,  size_t delay,  typename event< ResultType  ,  int >::ptr res_ev) {}      ResultType result;      int err;   };   stack_t _stack;   method_type_t _method;   bool is_onstack (const void *p) const   {     return (static_cast<const void *> (&_stack) <= p &&             static_cast<const void *> (&_stack + 1) > p);   } }; 
# 62 "tame_redundant.Th"
template< typename ResultType, typename TaskType >
void 
redundant_caller_manager_t< ResultType  ,  TaskType >::call( size_t __tame_index,  size_t __tame_delay,  typename event< ResultType  ,  int >::ptr __tame_res_ev, ptr<closure_t> __cls_g)
{
# 66 "tame_redundant.Th"

    
# 67 "tame_redundant.Th"
  redundant_caller_manager_t_ResultType_TaskType___call_9d10329b__closure_t< ResultType  ,  TaskType > *__cls;   ptr<redundant_caller_manager_t_ResultType_TaskType___call_9d10329b__closure_t< ResultType  ,  TaskType >  > __cls_r;   const char *__cls_type = "redundant_caller_manager_t_ResultType_TaskType___call_9d10329b__closure_t< ResultType  ,  TaskType > ";   use_reference (__cls_type);   if (!__cls_g) {     if (tame_check_leaks ()) start_rendezvous_collection ();     __cls_r = New refcounted<redundant_caller_manager_t_ResultType_TaskType___call_9d10329b__closure_t< ResultType  ,  TaskType > > (this, __tame_index, __tame_delay, __tame_res_ev);     if (tame_check_leaks ()) __cls_r->collect_rendezvous ();     __cls = __cls_r;     __cls_g = __cls_r;     __cls->set_method_pointer (&redundant_caller_manager_t< ResultType  ,  TaskType >::call);   } else {     __cls =     reinterpret_cast<redundant_caller_manager_t_ResultType_TaskType___call_9d10329b__closure_t< ResultType  ,  TaskType > *> (static_cast<closure_t *> (__cls_g));     __cls_r = mkref (__cls);   }   auto &result = __cls->_stack.result;   auto &err = __cls->_stack.err;   auto &index = __cls->_args.index; /*  size_t */   auto &delay = __cls->_args.delay; /*  size_t */   auto &res_ev = __cls->_args.res_ev; /*  typename event< ResultType  ,  int >::ptr */    use_reference (index);     use_reference (delay);     use_reference (res_ev);    switch (__cls->jumpto ()) {   case 0: break;   case 1:     goto redundant_caller_manager_t_ResultType_TaskType___call_9d10329b__label_1;     break;   default:     panic ("unexpected case.\n");     break;   }
# 70 "tame_redundant.Th"

    callers[index].set_delay(delay);
    
# 72 "tame_redundant.Th"
  do {     do {       closure_wrapper<redundant_caller_manager_t_ResultType_TaskType___call_9d10329b__closure_t< ResultType  ,  TaskType > > __cls_g (__cls_r);     __cls->init_block (1, 72);     __cls->set_jumpto (1); 
# 72 "tame_redundant.Th"

        (callers[index])(mkevent(result, err), this);
    
# 74 "tame_redundant.Th"
      if (!__cls->block_dec_count (__FL__))       return;     } while (0);  redundant_caller_manager_t_ResultType_TaskType___call_9d10329b__label_1:       ;   } while (0);
# 74 "tame_redundant.Th"

    res_ev->trigger(result, err);
# 76 "tame_redundant.Th"
  do {   __cls->end_of_scope_checks (76);   return;   } while (0);
# 76 "tame_redundant.Th"
}

template<typename ResultType, typename TaskType>
struct redundant_caller_t {
public:
    redundant_caller_t(size_t delay) 
        : m_delay(delay), m_ctask(0), m_timer(NULL), m_complete(false) { }
    
    redundant_caller_t() 
        : m_ctask(0), m_timer(NULL), m_complete(false) { }

    void operator()(typename event<ResultType, int>::ptr res_ev,
            redundant_caller_manager_t <ResultType, TaskType>* manager) {
        go (res_ev, manager);
    }
    
    void operator+=(const TaskType& t) {
        m_tasks.push_back(t);
    }

    void set_delay(size_t delay) {
        m_delay = delay;
    }
protected:
    
    void go(typename event<ResultType, int>::ptr res_ev,
            redundant_caller_manager_t <ResultType, TaskType>* manager,
            CLOSURE);
    void timer_wrapper(CLOSURE); 
    void task_wrapper(TaskType task,
            redundant_caller_manager_t <ResultType, TaskType>* manager,
            CLOSURE);

    size_t m_delay;
    size_t m_ctask;
    vec<TaskType> m_tasks;
    timecb_t *m_timer;
    typename event<bool, ResultType, int>::ptr m_monitor;
    bool m_complete;
};

//-----------------------------------------------------------------------------


# 120 "tame_redundant.Th"
template< typename ResultType, typename TaskType > class redundant_caller_t_ResultType_TaskType___task_wrapper_3f1679aa__closure_t : public closure_t { public:   redundant_caller_t_ResultType_TaskType___task_wrapper_3f1679aa__closure_t (redundant_caller_t< ResultType  ,  TaskType > *_self,  TaskType task,  redundant_caller_manager_t< ResultType  ,  TaskType > *manager) : closure_t ("tame_redundant.Th", "redundant_caller_t< ResultType  ,  TaskType >::task_wrapper", 122), _self (_self), _args (task, manager), _stack (_self, task, manager) {}   typedef void  (redundant_caller_t< ResultType  ,  TaskType >::*method_type_t) ( TaskType ,  redundant_caller_manager_t< ResultType  ,  TaskType > *, ptr<closure_t>);   void set_method_pointer (method_type_t m) { _method = m; }   void reenter ()   {     ((*_self).*_method)  (_args.task, _args.manager, mkref (this));   } void v_reenter () { reenter (); }   struct args_t {     args_t ( TaskType task,  redundant_caller_manager_t< ResultType  ,  TaskType > *manager) : task (task), manager (manager) {}      TaskType task;      redundant_caller_manager_t< ResultType  ,  TaskType > *manager;   };   redundant_caller_t< ResultType  ,  TaskType > *_self;   args_t _args;   struct stack_t {     stack_t (redundant_caller_t< ResultType  ,  TaskType > *_self,  TaskType task,  redundant_caller_manager_t< ResultType  ,  TaskType > *manager) : p_manager (NULL), err (-1)  {}      ResultType res;      ptr< redundant_caller_manager_t< ResultType  ,  TaskType > > p_manager;      int err;   };   stack_t _stack;   method_type_t _method;   bool is_onstack (const void *p) const   {     return (static_cast<const void *> (&_stack) <= p &&             static_cast<const void *> (&_stack + 1) > p);   } }; 
# 120 "tame_redundant.Th"
template< typename ResultType, typename TaskType >
void 
redundant_caller_t< ResultType  ,  TaskType >::task_wrapper( TaskType __tame_task,  redundant_caller_manager_t< ResultType  ,  TaskType > *__tame_manager, ptr<closure_t> __cls_g)
{
# 122 "tame_redundant.Th"

    
# 123 "tame_redundant.Th"
  redundant_caller_t_ResultType_TaskType___task_wrapper_3f1679aa__closure_t< ResultType  ,  TaskType > *__cls;   ptr<redundant_caller_t_ResultType_TaskType___task_wrapper_3f1679aa__closure_t< ResultType  ,  TaskType >  > __cls_r;   const char *__cls_type = "redundant_caller_t_ResultType_TaskType___task_wrapper_3f1679aa__closure_t< ResultType  ,  TaskType > ";   use_reference (__cls_type);   if (!__cls_g) {     if (tame_check_leaks ()) start_rendezvous_collection ();     __cls_r = New refcounted<redundant_caller_t_ResultType_TaskType___task_wrapper_3f1679aa__closure_t< ResultType  ,  TaskType > > (this, __tame_task, __tame_manager);     if (tame_check_leaks ()) __cls_r->collect_rendezvous ();     __cls = __cls_r;     __cls_g = __cls_r;     __cls->set_method_pointer (&redundant_caller_t< ResultType  ,  TaskType >::task_wrapper);   } else {     __cls =     reinterpret_cast<redundant_caller_t_ResultType_TaskType___task_wrapper_3f1679aa__closure_t< ResultType  ,  TaskType > *> (static_cast<closure_t *> (__cls_g));     __cls_r = mkref (__cls);   }   auto &res = __cls->_stack.res;   auto &p_manager = __cls->_stack.p_manager;   auto &err = __cls->_stack.err;   auto &task = __cls->_args.task; /*  TaskType */   auto &manager = __cls->_args.manager; /*  redundant_caller_manager_t< ResultType  ,  TaskType > **/    use_reference (task);     use_reference (manager);    switch (__cls->jumpto ()) {   case 0: break;   case 1:     goto redundant_caller_t_ResultType_TaskType___task_wrapper_3f1679aa__label_1;     break;   default:     panic ("unexpected case.\n");     break;   }
# 127 "tame_redundant.Th"

    
    if (manager) {
        p_manager = mkref(manager);
        p_manager->inc ();
    }
    
# 133 "tame_redundant.Th"
  do {     do {       closure_wrapper<redundant_caller_t_ResultType_TaskType___task_wrapper_3f1679aa__closure_t< ResultType  ,  TaskType > > __cls_g (__cls_r);     __cls->init_block (1, 133);     __cls->set_jumpto (1); 
# 133 "tame_redundant.Th"
 task(mkevent(res, err)); 
# 133 "tame_redundant.Th"
      if (!__cls->block_dec_count (__FL__))       return;     } while (0);  redundant_caller_t_ResultType_TaskType___task_wrapper_3f1679aa__label_1:       ;   } while (0);
# 133 "tame_redundant.Th"


    if (!m_complete)
        m_monitor->trigger(false, res, err);

    if (p_manager) {
        p_manager->dec();
    }
# 141 "tame_redundant.Th"
  do {   __cls->end_of_scope_checks (141);   return;   } while (0);
# 141 "tame_redundant.Th"
}

//-----------------------------------------------------------------------------

# 145 "tame_redundant.Th"
template< typename ResultType, typename TaskType > class redundant_caller_t_ResultType_TaskType___timer_wrapper_3f1679e8__closure_t : public closure_t { public:   redundant_caller_t_ResultType_TaskType___timer_wrapper_3f1679e8__closure_t (redundant_caller_t< ResultType  ,  TaskType > *_self) : closure_t ("tame_redundant.Th", "redundant_caller_t< ResultType  ,  TaskType >::timer_wrapper", 146), _self (_self), _args (), _stack (_self) {}   typedef void  (redundant_caller_t< ResultType  ,  TaskType >::*method_type_t) (ptr<closure_t>);   void set_method_pointer (method_type_t m) { _method = m; }   void reenter ()   {     ((*_self).*_method)  (mkref (this));   } void v_reenter () { reenter (); }   struct args_t {     args_t () {}   };   redundant_caller_t< ResultType  ,  TaskType > *_self;   args_t _args;   struct stack_t {     stack_t (redundant_caller_t< ResultType  ,  TaskType > *_self) {}   };   stack_t _stack;   method_type_t _method;   bool is_onstack (const void *p) const   {     return (static_cast<const void *> (&_stack) <= p &&             static_cast<const void *> (&_stack + 1) > p);   } }; 
# 145 "tame_redundant.Th"
template< typename ResultType, typename TaskType >
void 
redundant_caller_t< ResultType  ,  TaskType >::timer_wrapper(ptr<closure_t> __cls_g)
{
# 146 "tame_redundant.Th"
  redundant_caller_t_ResultType_TaskType___timer_wrapper_3f1679e8__closure_t< ResultType  ,  TaskType > *__cls;   ptr<redundant_caller_t_ResultType_TaskType___timer_wrapper_3f1679e8__closure_t< ResultType  ,  TaskType >  > __cls_r;   const char *__cls_type = "redundant_caller_t_ResultType_TaskType___timer_wrapper_3f1679e8__closure_t< ResultType  ,  TaskType > ";   use_reference (__cls_type);   if (!__cls_g) {     if (tame_check_leaks ()) start_rendezvous_collection ();     __cls_r = New refcounted<redundant_caller_t_ResultType_TaskType___timer_wrapper_3f1679e8__closure_t< ResultType  ,  TaskType > > (this);     if (tame_check_leaks ()) __cls_r->collect_rendezvous ();     __cls = __cls_r;     __cls_g = __cls_r;     __cls->set_method_pointer (&redundant_caller_t< ResultType  ,  TaskType >::timer_wrapper);   } else {     __cls =     reinterpret_cast<redundant_caller_t_ResultType_TaskType___timer_wrapper_3f1679e8__closure_t< ResultType  ,  TaskType > *> (static_cast<closure_t *> (__cls_g));     __cls_r = mkref (__cls);   }   switch (__cls->jumpto ()) {   case 0: break;   case 1:     goto redundant_caller_t_ResultType_TaskType___timer_wrapper_3f1679e8__label_1;     break;   default:     panic ("unexpected case.\n");     break;   }
# 146 "tame_redundant.Th"

    
# 147 "tame_redundant.Th"
  do {     do {       closure_wrapper<redundant_caller_t_ResultType_TaskType___timer_wrapper_3f1679e8__closure_t< ResultType  ,  TaskType > > __cls_g (__cls_r);     __cls->init_block (1, 147);     __cls->set_jumpto (1); 
# 147 "tame_redundant.Th"

        m_timer = delaycb(m_delay / 1000,
                (m_delay % 1000) * 1000000, mkevent());
    
# 150 "tame_redundant.Th"
      if (!__cls->block_dec_count (__FL__))       return;     } while (0);  redundant_caller_t_ResultType_TaskType___timer_wrapper_3f1679e8__label_1:       ;   } while (0);
# 150 "tame_redundant.Th"

    m_timer = NULL;
    m_monitor->trigger(true, ResultType(), 0);
# 153 "tame_redundant.Th"
  do {   __cls->end_of_scope_checks (153);   return;   } while (0);
# 153 "tame_redundant.Th"
}

//-----------------------------------------------------------------------------

# 157 "tame_redundant.Th"
template< typename ResultType, typename TaskType > class redundant_caller_t_ResultType_TaskType___go_3f167985__closure_t : public closure_t { public:   redundant_caller_t_ResultType_TaskType___go_3f167985__closure_t (redundant_caller_t< ResultType  ,  TaskType > *_self,  typename event< ResultType  ,  int >::ptr res_ev,  redundant_caller_manager_t< ResultType  ,  TaskType > *manager) : closure_t ("tame_redundant.Th", "redundant_caller_t< ResultType  ,  TaskType >::go", 160), _self (_self), _args (res_ev, manager), _stack (_self, res_ev, manager) {}   typedef void  (redundant_caller_t< ResultType  ,  TaskType >::*method_type_t) ( typename event< ResultType  ,  int >::ptr ,  redundant_caller_manager_t< ResultType  ,  TaskType > *, ptr<closure_t>);   void set_method_pointer (method_type_t m) { _method = m; }   void reenter ()   {     ((*_self).*_method)  (_args.res_ev, _args.manager, mkref (this));   } void v_reenter () { reenter (); }   struct args_t {     args_t ( typename event< ResultType  ,  int >::ptr res_ev,  redundant_caller_manager_t< ResultType  ,  TaskType > *manager) : res_ev (res_ev), manager (manager) {}      typename event< ResultType  ,  int >::ptr res_ev;      redundant_caller_manager_t< ResultType  ,  TaskType > *manager;   };   redundant_caller_t< ResultType  ,  TaskType > *_self;   args_t _args;   struct stack_t {     stack_t (redundant_caller_t< ResultType  ,  TaskType > *_self,  typename event< ResultType  ,  int >::ptr res_ev,  redundant_caller_manager_t< ResultType  ,  TaskType > *manager) : rv ("tame_redundant.Th", 163), timer (false)  {}      rendezvous_t<> rv;      bool timer;      ResultType result;      int err;   };   stack_t _stack;   method_type_t _method;   bool is_onstack (const void *p) const   {     return (static_cast<const void *> (&_stack) <= p &&             static_cast<const void *> (&_stack + 1) > p);   } }; 
# 157 "tame_redundant.Th"
template< typename ResultType, typename TaskType >
void 
redundant_caller_t< ResultType  ,  TaskType >::go( typename event< ResultType  ,  int >::ptr __tame_res_ev,  redundant_caller_manager_t< ResultType  ,  TaskType > *__tame_manager, ptr<closure_t> __cls_g)
{

    
# 162 "tame_redundant.Th"
  redundant_caller_t_ResultType_TaskType___go_3f167985__closure_t< ResultType  ,  TaskType > *__cls;   ptr<redundant_caller_t_ResultType_TaskType___go_3f167985__closure_t< ResultType  ,  TaskType >  > __cls_r;   const char *__cls_type = "redundant_caller_t_ResultType_TaskType___go_3f167985__closure_t< ResultType  ,  TaskType > ";   use_reference (__cls_type);   if (!__cls_g) {     if (tame_check_leaks ()) start_rendezvous_collection ();     __cls_r = New refcounted<redundant_caller_t_ResultType_TaskType___go_3f167985__closure_t< ResultType  ,  TaskType > > (this, __tame_res_ev, __tame_manager);     if (tame_check_leaks ()) __cls_r->collect_rendezvous ();     __cls = __cls_r;     __cls_g = __cls_r;     __cls->set_method_pointer (&redundant_caller_t< ResultType  ,  TaskType >::go);   } else {     __cls =     reinterpret_cast<redundant_caller_t_ResultType_TaskType___go_3f167985__closure_t< ResultType  ,  TaskType > *> (static_cast<closure_t *> (__cls_g));     __cls_r = mkref (__cls);   }   auto &rv = __cls->_stack.rv;   auto &timer = __cls->_stack.timer;   auto &result = __cls->_stack.result;   auto &err = __cls->_stack.err;   auto &res_ev = __cls->_args.res_ev; /*  typename event< ResultType  ,  int >::ptr */   auto &manager = __cls->_args.manager; /*  redundant_caller_manager_t< ResultType  ,  TaskType > **/    use_reference (res_ev);     use_reference (manager);    switch (__cls->jumpto ()) {   case 0: break;   case 1:     goto redundant_caller_t_ResultType_TaskType___go_3f167985__label_1;     break;   default:     panic ("unexpected case.\n");     break;   }
# 167 "tame_redundant.Th"


    if (!m_tasks.size()) { res_ev->trigger(result, 0); 
# 169 "tame_redundant.Th"
  do { __cls->end_of_scope_checks (169);
# 169 "tame_redundant.Th"
    return ;  } while (0); }


    m_monitor = mkevent(rv, timer, result, err);
    m_monitor->set_reuse(true);

    task_wrapper(m_tasks[m_ctask++], manager);
    timer_wrapper();
    while (true) {
        
# 178 "tame_redundant.Th"
redundant_caller_t_ResultType_TaskType___go_3f167985__label_1: do {   if (!(rv)._ti_next_trigger ()) {     __cls->set_jumpto (1);       (rv)._ti_set_join_cls (__cls_r);       return;   } else {     (rv)._ti_clear_join_method ();   } } while (0);
# 178 "tame_redundant.Th"

        if (timer) {
            if (m_ctask < m_tasks.size()) {
                task_wrapper(m_tasks[m_ctask++], manager);
                timer_wrapper();
            } else {
            }
        } else {
            break;
        }
    }
    if (m_timer) {
        timecb_remove(m_timer);
    }
    m_complete = true;
    rv.cancel();
    res_ev->trigger(result, err);
# 195 "tame_redundant.Th"
  do {   __cls->end_of_scope_checks (195);   return;   } while (0);
# 195 "tame_redundant.Th"
}

#endif
