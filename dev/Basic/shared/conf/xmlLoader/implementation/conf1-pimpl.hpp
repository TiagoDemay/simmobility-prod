// Not copyrighted - public domain.
//
// This sample parser implementation was generated by CodeSynthesis XSD,
// an XML Schema to C++ data binding compiler. You may use it in your
// programs without any restrictions.
//

#ifndef CONF1_PIMPL_HPP
#define CONF1_PIMPL_HPP

#include "../skeleton/conf1-pskel.hpp"

namespace sim_mob
{
  namespace conf
  {
    class model_pimpl: public virtual model_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      id (const ::std::string&);

      virtual void
      library (const ::std::string&);

      virtual void
      post_model ();
    };

    class workgroup_pimpl: public virtual workgroup_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      id (const ::std::string&);

      virtual void
      workers (int);

      virtual void
      post_workgroup ();
    };

    class distribution_pimpl: public virtual distribution_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      id (const ::std::string&);

      virtual void
      type (const ::std::string&);

      virtual void
      mean (int);

      virtual void
      stdev (int);

      virtual void
      post_distribution ();
    };

    class db_connection_pimpl: public virtual db_connection_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      param ();

      virtual void
      id (const ::std::string&);

      virtual void
      dbtype (const ::std::string&);

      virtual void
      post_db_connection ();
    };

    class db_param_pimpl: public virtual db_param_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      name (const ::std::string&);

      virtual void
      value (const ::std::string&);

      virtual void
      post_db_param ();
    };

    class db_proc_mapping_pimpl: public virtual db_proc_mapping_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      name (const ::std::string&);

      virtual void
      procedure (const ::std::string&);

      virtual void
      post_db_proc_mapping ();
    };

    class proc_map_pimpl: public virtual proc_map_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      mapping ();

      virtual void
      id (const ::std::string&);

      virtual void
      format (const ::std::string&);

      virtual void
      post_proc_map ();
    };

    class constructs_pimpl: public virtual constructs_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      models ();

      virtual void
      workgroups ();

      virtual void
      distributions ();

      virtual void
      db_connections ();

      virtual void
      db_proc_groups ();

      virtual void
      post_constructs ();
    };

    class SimMobility_pimpl: public virtual SimMobility_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      constructs ();

      virtual void
      post_SimMobility ();
    };

    class models_pimpl: public virtual models_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      lane_changing ();

      virtual void
      car_following ();

      virtual void
      intersection_driving ();

      virtual void
      sidewalk_movement ();

      virtual void
      post_models ();
    };

    class workgroups_pimpl: public virtual workgroups_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      workgroup ();

      virtual void
      post_workgroups ();
    };

    class distributions_pimpl: public virtual distributions_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      dist ();

      virtual void
      post_distributions ();
    };

    class db_connections_pimpl: public virtual db_connections_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      connection ();

      virtual void
      post_db_connections ();
    };

    class db_proc_groups_pimpl: public virtual db_proc_groups_pskel
    {
      public:
      virtual void
      pre ();

      virtual void
      proc_map ();

      virtual void
      post_db_proc_groups ();
    };
  }
}

#endif // CONF1_PIMPL_HPP
