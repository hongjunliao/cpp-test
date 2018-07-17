
# 1 "tame_connectors.Th"

// -*-c++-*-
/* $Id: tame.h 2077 2006-07-07 18:24:23Z max $ */

#ifndef _LIBTAME_CONNECTORS_H_
#define _LIBTAME_CONNECTORS_H_

#include "tame.h"

//
// Connectors that combine different events in useful ways.
//

typedef enum { OUTCOME_SUCC = 0,
	       OUTCOME_TIMEDOUT = 1,
	       OUTCOME_CANCELLED = 2,
	       OUTCOME_SIGNALED = 3,
	       OUTCOME_DISCARDED = 4 } outcome_t;

inline bool valid_timeout (int s, int ns)
{
  return (s >= 0 && ns >= 0 && (s > 0 || ns > 0));
}

template<class T1=void, class T2=void, class T3=void>
class connector_t {
public:

  connector_t () {}

  static typename event<T1,T2,T3>::ptr
  cnc (typename event<T1,T2,T3>::ref in, 
       _event_cancel_base *cobj,
       int to_s,
       int to_ns,
       outcome_t *ocp = NULL)
  {
    typename event<T1,T2,T3>::ptr  ncb;
    connector_t<T1,T2,T3> c;
    c.__cnc (&ncb, in, cobj, to_s, to_ns, ocp);
    return ncb;
  }

  static typename event<T1,T2,T3>::ptr
  sig (typename event<T1,T2,T3>::ref in, 
       vec<int> sigv,
       int *res,
       _event_cancel_base *cobj)
  {
    typename event<T1,T2,T3>::ptr  ncb;
    connector_t<T1,T2,T3> c;
    c.__sig (&ncb, in, sigv, res, cobj);
    return ncb;
  }

  static typename event<T1,T2,T3>::ptr
  first (typename event<T1,T2,T3>::ref *es, size_t sz)
  {
    typename event<T1,T2,T3>::ptr ncb;
    connector_t<T1,T2,T3> c;
    c.__first (es, sz);
    return ncb;
  }

  static typename event<T1,T2,T3>::ptr
  _klepto_timeout (ptr<closure_t> cls, typename event<T1,T2,T3>::ref in,
		  int to_s, int to_ns = 0,
		  outcome_t *ocp = NULL)
  {
    typename event<T1,T2,T3>::ptr ncb;
    connector_t<T1,T2,T3> c;
    c.__kto (&ncb, cls, in, to_s, to_ns, ocp);
    return ncb;
  }

  static typename event<T1,T2,T3>::ptr
  extend (evv_t ev, _tame_slot_set<T1,T2,T3> ss)
  {
    typename event<T1,T2,T3>::ptr ret;
    connector_t<T1,T2,T3> c;
    c.__extend (&ret, ev, ss);
    return ret;
  }

private:
  void __cnc (typename event<T1,T2,T3>::ptr *out, 
	      typename event<T1,T2,T3>::ref in, 
	      _event_cancel_base *c, 
	      int to_s, 
	      int to_ns,
	      outcome_t *ocp,
	      CLOSURE);

  void __kto (typename event<T1,T2,T3>::ptr *out, 
	      ptr<closure_t> c,
	      typename event<T1,T2,T3>::ref in, 
	      int to_s, 
	      int to_ns,
	      outcome_t *ocp,
	      CLOSURE);

  void __sig (typename event<T1,T2,T3>::ptr *out, 
	      typename event<T1,T2,T3>::ref in, 
	      vec<int> sigs,
	      int *sigp,
	      _event_cancel_base *cobj, 
	      CLOSURE);

  void __extend (typename event<T1,T2,T3>::ptr *out,
		 evv_t in,
		 _tame_slot_set<T1,T2,T3> ss,
		 CLOSURE);

};

# 116 "tame_connectors.Th"
template< class T1, class T2, class T3 > class connector_t_T1_T2_T3_____sig_442da14d__closure_t : public closure_t { public:   connector_t_T1_T2_T3_____sig_442da14d__closure_t (connector_t< T1  ,  T2  ,  T3 > *_self,  typename event< T1  ,  T2  ,  T3 >::ptr *out,  typename event< T1  ,  T2  ,  T3 >::ref in,  vec< int > sigs,  int *sigp,  _event_cancel_base *cobj) : closure_t ("tame_connectors.Th", "connector_t< T1  ,  T2  ,  T3 >::__sig", 122), _self (_self), _args (out, in, sigs, sigp, cobj), _stack (_self, out, in, sigs, sigp, cobj) {}   typedef void  (connector_t< T1  ,  T2  ,  T3 >::*method_type_t) ( typename event< T1  ,  T2  ,  T3 >::ptr *,  typename event< T1  ,  T2  ,  T3 >::ref ,  vec< int > ,  int *,  _event_cancel_base *, ptr<closure_t>);   void set_method_pointer (method_type_t m) { _method = m; }   void reenter ()   {     ((*_self).*_method)  (_args.out, _args.in, _args.sigs, _args.sigp, _args.cobj, mkref (this));   } void v_reenter () { reenter (); }   struct args_t {     args_t ( typename event< T1  ,  T2  ,  T3 >::ptr *out,  typename event< T1  ,  T2  ,  T3 >::ref in,  vec< int > sigs,  int *sigp,  _event_cancel_base *cobj) : out (out), in (in), sigs (sigs), sigp (sigp), cobj (cobj) {}      typename event< T1  ,  T2  ,  T3 >::ptr *out;      typename event< T1  ,  T2  ,  T3 >::ref in;      vec< int > sigs;      int *sigp;      _event_cancel_base *cobj;   };   connector_t< T1  ,  T2  ,  T3 > *_self;   args_t _args;   struct stack_t {     stack_t (connector_t< T1  ,  T2  ,  T3 > *_self,  typename event< T1  ,  T2  ,  T3 >::ptr *out,  typename event< T1  ,  T2  ,  T3 >::ref in,  vec< int > sigs,  int *sigp,  _event_cancel_base *cobj) : rv ("tame_connectors.Th", 124)  {}      rendezvous_t< int > rv;      int i;      int sig;   };   stack_t _stack;   method_type_t _method;   bool is_onstack (const void *p) const   {     return (static_cast<const void *> (&_stack) <= p &&             static_cast<const void *> (&_stack + 1) > p);   } }; 
# 116 "tame_connectors.Th"
template< class T1, class T2, class T3 >
void 
connector_t< T1  ,  T2  ,  T3 >::__sig( typename event< T1  ,  T2  ,  T3 >::ptr *__tame_out,  typename event< T1  ,  T2  ,  T3 >::ref __tame_in,  vec< int > __tame_sigs,  int *__tame_sigp,  _event_cancel_base *__tame_cobj, ptr<closure_t> __cls_g)
{
# 122 "tame_connectors.Th"

  
# 123 "tame_connectors.Th"
  connector_t_T1_T2_T3_____sig_442da14d__closure_t< T1  ,  T2  ,  T3 > *__cls;   ptr<connector_t_T1_T2_T3_____sig_442da14d__closure_t< T1  ,  T2  ,  T3 >  > __cls_r;   const char *__cls_type = "connector_t_T1_T2_T3_____sig_442da14d__closure_t< T1  ,  T2  ,  T3 > ";   use_reference (__cls_type);   if (!__cls_g) {     if (tame_check_leaks ()) start_rendezvous_collection ();     __cls_r = New refcounted<connector_t_T1_T2_T3_____sig_442da14d__closure_t< T1  ,  T2  ,  T3 > > (this, __tame_out, __tame_in, __tame_sigs, __tame_sigp, __tame_cobj);     if (tame_check_leaks ()) __cls_r->collect_rendezvous ();     __cls = __cls_r;     __cls_g = __cls_r;     __cls->set_method_pointer (&connector_t< T1  ,  T2  ,  T3 >::__sig);   } else {     __cls =     reinterpret_cast<connector_t_T1_T2_T3_____sig_442da14d__closure_t< T1  ,  T2  ,  T3 > *> (static_cast<closure_t *> (__cls_g));     __cls_r = mkref (__cls);   }   auto &rv = __cls->_stack.rv;   auto &i = __cls->_stack.i;   auto &sig = __cls->_stack.sig;   auto &out = __cls->_args.out; /*  typename event< T1  ,  T2  ,  T3 >::ptr **/   auto &in = __cls->_args.in; /*  typename event< T1  ,  T2  ,  T3 >::ref */   auto &sigs = __cls->_args.sigs; /*  vec< int > */   auto &sigp = __cls->_args.sigp; /*  int **/   auto &cobj = __cls->_args.cobj; /*  _event_cancel_base **/    use_reference (out);     use_reference (in);     use_reference (sigs);     use_reference (sigp);     use_reference (cobj);    switch (__cls->jumpto ()) {   case 0: break;   case 1:     goto connector_t_T1_T2_T3_____sig_442da14d__label_1;     break;   default:     panic ("unexpected case.\n");     break;   }
# 127 "tame_connectors.Th"


  if (cobj) {
    cobj->set_cancel_notifier (mkevent (rv, -1));
  }
  for (i = 0; i < sigs.size (); i++) {
    sigcb (sigs[i], mkevent (rv, sigs[i]));
  }
  *out = mkevent_rs (in->slot_set (), rv, 0);
  (*out)->set_cancel_notifier (mkevent (rv, -2));

  
# 138 "tame_connectors.Th"
connector_t_T1_T2_T3_____sig_442da14d__label_1: do {   if (!(rv)._ti_next_trigger (sig)) {     __cls->set_jumpto (1);       (rv)._ti_set_join_cls (__cls_r);       return;   } else {     (rv)._ti_clear_join_method ();   } } while (0);
# 138 "tame_connectors.Th"


  for (i = 0; i < sigs.size (); i++) {
    sigcb (sigs[i], NULL);
  }

  rv.cancel ();
  if (sigp) *sigp = sig;

  in->trigger_no_assign ();
# 148 "tame_connectors.Th"
  do {   __cls->end_of_scope_checks (148);   return;   } while (0);
# 148 "tame_connectors.Th"
}

# 150 "tame_connectors.Th"
template< class T1, class T2, class T3 > class connector_t_T1_T2_T3_____kto_442da16d__closure_t : public closure_t { public:   connector_t_T1_T2_T3_____kto_442da16d__closure_t (connector_t< T1  ,  T2  ,  T3 > *_self,  typename event< T1  ,  T2  ,  T3 >::ptr *out,  ptr< closure_t > c,  typename event< T1  ,  T2  ,  T3 >::ref in,  int to_s,  int to_ns,  outcome_t *ocp) : closure_t ("tame_connectors.Th", "connector_t< T1  ,  T2  ,  T3 >::__kto", 157), _self (_self), _args (out, c, in, to_s, to_ns, ocp), _stack (_self, out, c, in, to_s, to_ns, ocp) {}   typedef void  (connector_t< T1  ,  T2  ,  T3 >::*method_type_t) ( typename event< T1  ,  T2  ,  T3 >::ptr *,  ptr< closure_t > ,  typename event< T1  ,  T2  ,  T3 >::ref ,  int ,  int ,  outcome_t *, ptr<closure_t>);   void set_method_pointer (method_type_t m) { _method = m; }   void reenter ()   {     ((*_self).*_method)  (_args.out, _args.c, _args.in, _args.to_s, _args.to_ns, _args.ocp, mkref (this));   } void v_reenter () { reenter (); }   struct args_t {     args_t ( typename event< T1  ,  T2  ,  T3 >::ptr *out,  ptr< closure_t > c,  typename event< T1  ,  T2  ,  T3 >::ref in,  int to_s,  int to_ns,  outcome_t *ocp) : out (out), c (c), in (in), to_s (to_s), to_ns (to_ns), ocp (ocp) {}      typename event< T1  ,  T2  ,  T3 >::ptr *out;      ptr< closure_t > c;      typename event< T1  ,  T2  ,  T3 >::ref in;      int to_s;      int to_ns;      outcome_t *ocp;   };   connector_t< T1  ,  T2  ,  T3 > *_self;   args_t _args;   struct stack_t {     stack_t (connector_t< T1  ,  T2  ,  T3 > *_self,  typename event< T1  ,  T2  ,  T3 >::ptr *out,  ptr< closure_t > c,  typename event< T1  ,  T2  ,  T3 >::ref in,  int to_s,  int to_ns,  outcome_t *ocp) : rv ("tame_connectors.Th", 159), tcb (NULL)  {}      rendezvous_t< outcome_t > rv;      outcome_t outc;      timecb_t *tcb;   };   stack_t _stack;   method_type_t _method;   bool is_onstack (const void *p) const   {     return (static_cast<const void *> (&_stack) <= p &&             static_cast<const void *> (&_stack + 1) > p);   } }; 
# 150 "tame_connectors.Th"
template< class T1, class T2, class T3 >
void 
connector_t< T1  ,  T2  ,  T3 >::__kto( typename event< T1  ,  T2  ,  T3 >::ptr *__tame_out,  ptr< closure_t > __tame_c,  typename event< T1  ,  T2  ,  T3 >::ref __tame_in,  int __tame_to_s,  int __tame_to_ns,  outcome_t *__tame_ocp, ptr<closure_t> __cls_g)
{
# 157 "tame_connectors.Th"

  
# 158 "tame_connectors.Th"
  connector_t_T1_T2_T3_____kto_442da16d__closure_t< T1  ,  T2  ,  T3 > *__cls;   ptr<connector_t_T1_T2_T3_____kto_442da16d__closure_t< T1  ,  T2  ,  T3 >  > __cls_r;   const char *__cls_type = "connector_t_T1_T2_T3_____kto_442da16d__closure_t< T1  ,  T2  ,  T3 > ";   use_reference (__cls_type);   if (!__cls_g) {     if (tame_check_leaks ()) start_rendezvous_collection ();     __cls_r = New refcounted<connector_t_T1_T2_T3_____kto_442da16d__closure_t< T1  ,  T2  ,  T3 > > (this, __tame_out, __tame_c, __tame_in, __tame_to_s, __tame_to_ns, __tame_ocp);     if (tame_check_leaks ()) __cls_r->collect_rendezvous ();     __cls = __cls_r;     __cls_g = __cls_r;     __cls->set_method_pointer (&connector_t< T1  ,  T2  ,  T3 >::__kto);   } else {     __cls =     reinterpret_cast<connector_t_T1_T2_T3_____kto_442da16d__closure_t< T1  ,  T2  ,  T3 > *> (static_cast<closure_t *> (__cls_g));     __cls_r = mkref (__cls);   }   auto &rv = __cls->_stack.rv;   auto &outc = __cls->_stack.outc;   auto &tcb = __cls->_stack.tcb;   auto &out = __cls->_args.out; /*  typename event< T1  ,  T2  ,  T3 >::ptr **/   auto &c = __cls->_args.c; /*  ptr< closure_t > */   auto &in = __cls->_args.in; /*  typename event< T1  ,  T2  ,  T3 >::ref */   auto &to_s = __cls->_args.to_s; /*  int */   auto &to_ns = __cls->_args.to_ns; /*  int */   auto &ocp = __cls->_args.ocp; /*  outcome_t **/    use_reference (out);     use_reference (c);     use_reference (in);     use_reference (to_s);     use_reference (to_ns);     use_reference (ocp);    switch (__cls->jumpto ()) {   case 0: break;   case 1:     goto connector_t_T1_T2_T3_____kto_442da16d__label_1;     break;   case 2:     goto connector_t_T1_T2_T3_____kto_442da16d__label_2;     break;   default:     panic ("unexpected case.\n");     break;   }
# 162 "tame_connectors.Th"

  tcb = delaycb (to_s, to_ns, mkevent (rv, OUTCOME_TIMEDOUT));
  *out = mkevent_rs (in->slot_set (), rv, OUTCOME_SUCC);
  
# 165 "tame_connectors.Th"
connector_t_T1_T2_T3_____kto_442da16d__label_1: do {   if (!(rv)._ti_next_trigger (outc)) {     __cls->set_jumpto (1);       (rv)._ti_set_join_cls (__cls_r);       return;   } else {     (rv)._ti_clear_join_method ();   } } while (0);
# 165 "tame_connectors.Th"


  if (ocp) *ocp = outc;
  if (outc != OUTCOME_TIMEDOUT) {
    assert (tcb);
    timecb_remove (tcb);
    rv.cancel ();
    in->trigger_no_assign ();
  } else {
    // the timer fired first!
    assert (outc == OUTCOME_TIMEDOUT);
    tcb = NULL;  
    in->trigger_no_assign ();
    
# 178 "tame_connectors.Th"
connector_t_T1_T2_T3_____kto_442da16d__label_2: do {   if (!(rv)._ti_next_trigger (outc)) {     __cls->set_jumpto (2);       (rv)._ti_set_join_cls (__cls_r);       return;   } else {     (rv)._ti_clear_join_method ();   } } while (0);
# 178 "tame_connectors.Th"

    assert (outc == OUTCOME_SUCC);
  }
# 181 "tame_connectors.Th"
  do {   __cls->end_of_scope_checks (181);   return;   } while (0);
# 181 "tame_connectors.Th"
}

# 183 "tame_connectors.Th"
template< class T1, class T2, class T3 > class connector_t_T1_T2_T3_____extend_442da083__closure_t : public closure_t { public:   connector_t_T1_T2_T3_____extend_442da083__closure_t (connector_t< T1  ,  T2  ,  T3 > *_self,  typename event< T1  ,  T2  ,  T3 >::ptr *out,  evv_t in,  _tame_slot_set< T1  ,  T2  ,  T3 > ss) : closure_t ("tame_connectors.Th", "connector_t< T1  ,  T2  ,  T3 >::__extend", 186), _self (_self), _args (out, in, ss), _stack (_self, out, in, ss) {}   typedef void  (connector_t< T1  ,  T2  ,  T3 >::*method_type_t) ( typename event< T1  ,  T2  ,  T3 >::ptr *,  evv_t ,  _tame_slot_set< T1  ,  T2  ,  T3 > , ptr<closure_t>);   void set_method_pointer (method_type_t m) { _method = m; }   void reenter ()   {     ((*_self).*_method)  (_args.out, _args.in, _args.ss, mkref (this));   } void v_reenter () { reenter (); }   struct args_t {     args_t ( typename event< T1  ,  T2  ,  T3 >::ptr *out,  evv_t in,  _tame_slot_set< T1  ,  T2  ,  T3 > ss) : out (out), in (in), ss (ss) {}      typename event< T1  ,  T2  ,  T3 >::ptr *out;      evv_t in;      _tame_slot_set< T1  ,  T2  ,  T3 > ss;   };   connector_t< T1  ,  T2  ,  T3 > *_self;   args_t _args;   struct stack_t {     stack_t (connector_t< T1  ,  T2  ,  T3 > *_self,  typename event< T1  ,  T2  ,  T3 >::ptr *out,  evv_t in,  _tame_slot_set< T1  ,  T2  ,  T3 > ss) {}   };   stack_t _stack;   method_type_t _method;   bool is_onstack (const void *p) const   {     return (static_cast<const void *> (&_stack) <= p &&             static_cast<const void *> (&_stack + 1) > p);   } }; 
# 183 "tame_connectors.Th"
template< class T1, class T2, class T3 >
void 
connector_t< T1  ,  T2  ,  T3 >::__extend( typename event< T1  ,  T2  ,  T3 >::ptr *__tame_out,  evv_t __tame_in,  _tame_slot_set< T1  ,  T2  ,  T3 > __tame_ss, ptr<closure_t> __cls_g)
{
# 186 "tame_connectors.Th"
  connector_t_T1_T2_T3_____extend_442da083__closure_t< T1  ,  T2  ,  T3 > *__cls;   ptr<connector_t_T1_T2_T3_____extend_442da083__closure_t< T1  ,  T2  ,  T3 >  > __cls_r;   const char *__cls_type = "connector_t_T1_T2_T3_____extend_442da083__closure_t< T1  ,  T2  ,  T3 > ";   use_reference (__cls_type);   if (!__cls_g) {     if (tame_check_leaks ()) start_rendezvous_collection ();     __cls_r = New refcounted<connector_t_T1_T2_T3_____extend_442da083__closure_t< T1  ,  T2  ,  T3 > > (this, __tame_out, __tame_in, __tame_ss);     if (tame_check_leaks ()) __cls_r->collect_rendezvous ();     __cls = __cls_r;     __cls_g = __cls_r;     __cls->set_method_pointer (&connector_t< T1  ,  T2  ,  T3 >::__extend);   } else {     __cls =     reinterpret_cast<connector_t_T1_T2_T3_____extend_442da083__closure_t< T1  ,  T2  ,  T3 > *> (static_cast<closure_t *> (__cls_g));     __cls_r = mkref (__cls);   }   auto &out = __cls->_args.out; /*  typename event< T1  ,  T2  ,  T3 >::ptr **/   auto &in = __cls->_args.in; /*  evv_t */   auto &ss = __cls->_args.ss; /*  _tame_slot_set< T1  ,  T2  ,  T3 > */    use_reference (out);     use_reference (in);     use_reference (ss);    switch (__cls->jumpto ()) {   case 0: break;   case 1:     goto connector_t_T1_T2_T3_____extend_442da083__label_1;     break;   default:     panic ("unexpected case.\n");     break;   }
# 186 "tame_connectors.Th"

  
# 187 "tame_connectors.Th"
  do {     do {       closure_wrapper<connector_t_T1_T2_T3_____extend_442da083__closure_t< T1  ,  T2  ,  T3 > > __cls_g (__cls_r);     __cls->init_block (1, 187);     __cls->set_jumpto (1); 
# 187 "tame_connectors.Th"
 *out = mkevent_rs (ss); 
# 187 "tame_connectors.Th"
      if (!__cls->block_dec_count (__FL__))       return;     } while (0);  connector_t_T1_T2_T3_____extend_442da083__label_1:       ;   } while (0);
# 187 "tame_connectors.Th"

  in->trigger ();
# 189 "tame_connectors.Th"
  do {   __cls->end_of_scope_checks (189);   return;   } while (0);
# 189 "tame_connectors.Th"
}

# 191 "tame_connectors.Th"
template< class T1, class T2, class T3 > class connector_t_T1_T2_T3_____cnc_442da0e0__closure_t : public closure_t { public:   connector_t_T1_T2_T3_____cnc_442da0e0__closure_t (connector_t< T1  ,  T2  ,  T3 > *_self,  typename event< T1  ,  T2  ,  T3 >::ptr *out,  typename event< T1  ,  T2  ,  T3 >::ref in,  _event_cancel_base *cobj,  int to_s,  int to_ns,  outcome_t *ocp) : closure_t ("tame_connectors.Th", "connector_t< T1  ,  T2  ,  T3 >::__cnc", 198), _self (_self), _args (out, in, cobj, to_s, to_ns, ocp), _stack (_self, out, in, cobj, to_s, to_ns, ocp) {}   typedef void  (connector_t< T1  ,  T2  ,  T3 >::*method_type_t) ( typename event< T1  ,  T2  ,  T3 >::ptr *,  typename event< T1  ,  T2  ,  T3 >::ref ,  _event_cancel_base *,  int ,  int ,  outcome_t *, ptr<closure_t>);   void set_method_pointer (method_type_t m) { _method = m; }   void reenter ()   {     ((*_self).*_method)  (_args.out, _args.in, _args.cobj, _args.to_s, _args.to_ns, _args.ocp, mkref (this));   } void v_reenter () { reenter (); }   struct args_t {     args_t ( typename event< T1  ,  T2  ,  T3 >::ptr *out,  typename event< T1  ,  T2  ,  T3 >::ref in,  _event_cancel_base *cobj,  int to_s,  int to_ns,  outcome_t *ocp) : out (out), in (in), cobj (cobj), to_s (to_s), to_ns (to_ns), ocp (ocp) {}      typename event< T1  ,  T2  ,  T3 >::ptr *out;      typename event< T1  ,  T2  ,  T3 >::ref in;      _event_cancel_base *cobj;      int to_s;      int to_ns;      outcome_t *ocp;   };   connector_t< T1  ,  T2  ,  T3 > *_self;   args_t _args;   struct stack_t {     stack_t (connector_t< T1  ,  T2  ,  T3 > *_self,  typename event< T1  ,  T2  ,  T3 >::ptr *out,  typename event< T1  ,  T2  ,  T3 >::ref in,  _event_cancel_base *cobj,  int to_s,  int to_ns,  outcome_t *ocp) : rv ("tame_connectors.Th", 200), tcb (NULL)  {}      rendezvous_t< outcome_t > rv;      outcome_t outc;      timecb_t *tcb;   };   stack_t _stack;   method_type_t _method;   bool is_onstack (const void *p) const   {     return (static_cast<const void *> (&_stack) <= p &&             static_cast<const void *> (&_stack + 1) > p);   } }; 
# 191 "tame_connectors.Th"
template< class T1, class T2, class T3 >
void 
connector_t< T1  ,  T2  ,  T3 >::__cnc( typename event< T1  ,  T2  ,  T3 >::ptr *__tame_out,  typename event< T1  ,  T2  ,  T3 >::ref __tame_in,  _event_cancel_base *__tame_cobj,  int __tame_to_s,  int __tame_to_ns,  outcome_t *__tame_ocp, ptr<closure_t> __cls_g)
{
# 198 "tame_connectors.Th"

  
# 199 "tame_connectors.Th"
  connector_t_T1_T2_T3_____cnc_442da0e0__closure_t< T1  ,  T2  ,  T3 > *__cls;   ptr<connector_t_T1_T2_T3_____cnc_442da0e0__closure_t< T1  ,  T2  ,  T3 >  > __cls_r;   const char *__cls_type = "connector_t_T1_T2_T3_____cnc_442da0e0__closure_t< T1  ,  T2  ,  T3 > ";   use_reference (__cls_type);   if (!__cls_g) {     if (tame_check_leaks ()) start_rendezvous_collection ();     __cls_r = New refcounted<connector_t_T1_T2_T3_____cnc_442da0e0__closure_t< T1  ,  T2  ,  T3 > > (this, __tame_out, __tame_in, __tame_cobj, __tame_to_s, __tame_to_ns, __tame_ocp);     if (tame_check_leaks ()) __cls_r->collect_rendezvous ();     __cls = __cls_r;     __cls_g = __cls_r;     __cls->set_method_pointer (&connector_t< T1  ,  T2  ,  T3 >::__cnc);   } else {     __cls =     reinterpret_cast<connector_t_T1_T2_T3_____cnc_442da0e0__closure_t< T1  ,  T2  ,  T3 > *> (static_cast<closure_t *> (__cls_g));     __cls_r = mkref (__cls);   }   auto &rv = __cls->_stack.rv;   auto &outc = __cls->_stack.outc;   auto &tcb = __cls->_stack.tcb;   auto &out = __cls->_args.out; /*  typename event< T1  ,  T2  ,  T3 >::ptr **/   auto &in = __cls->_args.in; /*  typename event< T1  ,  T2  ,  T3 >::ref */   auto &cobj = __cls->_args.cobj; /*  _event_cancel_base **/   auto &to_s = __cls->_args.to_s; /*  int */   auto &to_ns = __cls->_args.to_ns; /*  int */   auto &ocp = __cls->_args.ocp; /*  outcome_t **/    use_reference (out);     use_reference (in);     use_reference (cobj);     use_reference (to_s);     use_reference (to_ns);     use_reference (ocp);    switch (__cls->jumpto ()) {   case 0: break;   case 1:     goto connector_t_T1_T2_T3_____cnc_442da0e0__label_1;     break;   default:     panic ("unexpected case.\n");     break;   }
# 203 "tame_connectors.Th"


  if (cobj) {
    cobj->set_cancel_notifier (mkevent (rv, OUTCOME_CANCELLED)); 
  }

  if (valid_timeout (to_s, to_ns)) {
    tcb = delaycb (to_s, to_ns, mkevent (rv, OUTCOME_TIMEDOUT));
  }

  *out = mkevent_rs (in->slot_set (), rv, OUTCOME_SUCC);
  (*out)->set_cancel_notifier (mkevent (rv, OUTCOME_DISCARDED));
  
# 215 "tame_connectors.Th"
connector_t_T1_T2_T3_____cnc_442da0e0__label_1: do {   if (!(rv)._ti_next_trigger (outc)) {     __cls->set_jumpto (1);       (rv)._ti_set_join_cls (__cls_r);       return;   } else {     (rv)._ti_clear_join_method ();   } } while (0);
# 215 "tame_connectors.Th"


  if (outc != OUTCOME_TIMEDOUT && tcb) {
    timecb_remove (tcb);
  }
  tcb = NULL;

  // Cancel before issuing the trigger, for safety's sake.
  rv.cancel ();

  if (ocp) *ocp = outc;

  in->trigger_no_assign ();
# 228 "tame_connectors.Th"
  do {   __cls->end_of_scope_checks (228);   return;   } while (0);
# 228 "tame_connectors.Th"
}

namespace connector {

  template<typename T1, typename T2, typename T3>
  ref<_event<T1,T2,T3> >
  cnc (ref<_event<T1,T2,T3> > in, _event_cancel_base *cobj, 
	outcome_t *ocp = NULL)
  {
    return connector_t<T1,T2,T3>::cnc (in, cobj, -1, -1, ocp);
  }

  template<typename T1, typename T2, typename T3>
  ref<_event<T1,T2,T3> >
  cnc (ref<_event<T1,T2,T3> > in, _event_cancel_base *cobj, 
       int to_s, int to_ns,
	outcome_t *ocp = NULL)
  {
    return connector_t<T1,T2,T3>::cnc (in, cobj, to_s, to_ns, ocp);
  }

  template<typename T1, typename T2, typename T3>
  ref<_event<T1,T2,T3> >
  timeout (ref<_event<T1,T2,T3> > in, int to_s, int to_ns = 0, 
	   outcome_t *ocp = NULL)
  {
    return connector_t<T1,T2,T3>::cnc (in, NULL, to_s, to_ns, ocp);
  }

  template<typename T1, typename T2, typename T3>
  ref<_event<T1,T2,T3> >
  _klepto_timeout (ptr<closure_t> cl, ref<_event<T1,T2,T3> > in,
		  int to_s, int to_ns = 0,
		  outcome_t *ocp = NULL)
  {
    return connector_t<T1,T2,T3>::_klepto_timeout (cl, in, to_s, to_ns, ocp);
  }

  template<typename T1, typename T2, typename T3>
  ref<_event<T1,T2,T3> >
  sig (ref<_event<T1,T2,T3> > in, vec<int> sigs, int *sigp = NULL,
       _event_cancel_base *cobj = NULL)
  {
    return connector_t<T1,T2,T3>::sig (in, sigs, sigp, cobj);
  }

  template<typename T1, typename T2, typename T3>
  ref<_event<T1,T2,T3> >
  first (ref<_event<T1,T2,T3> > *es, size_t sz)
  {
    return connector_t<T1,T2,T3>::first (es, sz);
  }

  evb_t tand (evv_t in, bool *b);
  evb_t tor (evv_t in, bool *b);

  // Extend a given void event into an event that, when triggered,
  // sets the passed references.  Same for the next three calls.
  template<typename T1> ref<_event<T1> >
  extend (evv_t ev, T1 &t1)
  {
    _tame_slot_set<T1> ss (&t1);
    return connector_t<T1>::extend (ev, ss);
  }

  template<typename T1, typename T2> ref<_event<T1,T2> >
  extend (evv_t ev, T1 &t1, T2 &t2)
  {
    _tame_slot_set<T1,T2> ss (&t1, &t2);
    return connector_t<T1,T2>::extend (ev, ss);
  }

  template<typename T1, typename T2, typename T3> 
  ref<_event<T1,T2,T3> >
  extend (evv_t ev, T1 &t1, T2 &t2, T3 &t3)
  {
    _tame_slot_set<T1,T2,T3> ss (&t1, &t2, &t3);
    return connector_t<T1,T2,T3>::extend (ev, ss);
  }

};

//
// klepto timeout grabs onto the current closure and holds onto it,
// until the call in question fires, even if the timeout fires first.
//
#define klepto_timeout(...) _klepto_timeout(__cls_r, ## __VA_ARGS__ )

#endif /* _LIBTAME_CONNECTORS_H_ */
