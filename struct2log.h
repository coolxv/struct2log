#ifndef _STRUCT2LOG_H_
#define _STRUCT2LOG_H_

//for C
#include <unistd.h>
#include <ctime>
#include <cstdio>
#include <cstdint>
//for container
#include <tuple>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
//for lock
#include <mutex>
//for serialization
#include <sstream>
//for file
#include <fstream>
//for type traits
#include <type_traits>
//for std::shared_ptr
#include <memory>
//for base type
#include <cinttypes>
//for SQL connector
#include <thread>
//for extend
#include "dlfcn.h"

//Public Macro
#define STRUCT2LOG(_TABLE_NAME_, ...)\
private:\
   constexpr static const char *__table_name = #_TABLE_NAME_;\
   constexpr static const char *__field_names = #__VA_ARGS__;\
private:\
    friend class struct2log_impl::injection_helper;\
private:\
    inline std::vector<std::string> &__types(std::vector<std::string> &vec)\
    { return struct2log_impl::injection_helper::serialize_type (vec, __VA_ARGS__); }\
    inline std::vector<std::string> &__types(std::vector<std::string> &vec) const\
    { return struct2log_impl::injection_helper::serialize_type (vec, __VA_ARGS__); }\
    inline std::vector<std::string> &__values(std::vector<std::string> &vec)\
    { return struct2log_impl::injection_helper::serialize_value (vec, __VA_ARGS__); }\
    inline std::vector<std::string> &__values(std::vector<std::string> &vec) const\
    { return struct2log_impl::injection_helper::serialize_value (vec, __VA_ARGS__); }\
private:\
    constexpr static const char *__key_field_name = "_aid";\
    inline std::vector<std::string> &__pk_type(std::vector<std::string> &vec)\
    { return struct2log_impl::injection_helper::serialize_type (vec, (unsigned long)this); }\
    inline std::vector<std::string> &__pk_type(std::vector<std::string> &vec) const\
    { return struct2log_impl::injection_helper::serialize_type (vec, (unsigned long)this); }\
    inline std::vector<std::string> &__pk_value(std::vector<std::string> &vec)\
    { return struct2log_impl::injection_helper::serialize_value (vec, (unsigned long)this); }\
    inline std::vector<std::string> &__pk_value(std::vector<std::string> &vec) const\
    { return struct2log_impl::injection_helper::serialize_value (vec, (unsigned long)this); }\


//Private Macros
#ifdef STRUCT2LOG_DEBUG 
#define STRUCT2LOG_DEBUG_EXCEPTION(e) std::cerr << "[exception]" << e.what() << std::endl;
#define STRUCT2LOG_DEBUG_ERROR(e) std::cerr << "[error]" << e << std::endl;
#define STRUCT2LOG_DEBUG_INFO(i) std::cerr << "[info]" << i << std::endl;
#else
#define STRUCT2LOG_DEBUG_EXCEPTION(e)
#define STRUCT2LOG_DEBUG_ERROR(e)
#define STRUCT2LOG_DEBUG_INFO(i)
#endif


#define STRUCT2LOG_BUF_SIZE 1024
#define STRUCT2LOG_PROCESS_NAEME_SIZE 128
#define STRUCT2LOG_DATE_BUF_SIZE 64
#define STRUCT2LOG_LOG_FILE "./%s_%s_%s_%s.log"
#define STRUCT2LOG_EXT_CONNECTOR "STRUCT2LOG_EXT_CONNECTOR"
#define STRUCT2LOG_EXT_CONNECTOR_INIT "__connector_init__"
#define STRUCT2LOG_EXT_LOG_PATH_PATTERN "STRUCT2LOG_EXT_LOG_PATH_PATTERN"



namespace struct2log_impl
{
    class struct2log_connector
    {
    public:
        struct2log_connector (const std::string &name):connection_(name){}
        virtual ~struct2log_connector (){}
        virtual void execute (const std::ostringstream &log) = 0;
    protected:
        std::string connection_;
    };

    class file_connector : public struct2log_connector
    {
    public:
        file_connector (const std::string &name)
            :struct2log_connector(name)
        {
            connection_ = generate_file_name();
            file_.open(connection_, std::ofstream::trunc);
            STRUCT2LOG_DEBUG_INFO("open file " + connection_);
        }
        virtual ~file_connector ()
        {
            if(file_.is_open())
            {
                file_.close();
                STRUCT2LOG_DEBUG_INFO("close file " + connection_);
            }
        }
        virtual void execute (const std::ostringstream &log)
        {
            if(file_.is_open())
            {
                mtx_.lock();
                file_ << log.str();
                file_.flush();
                mtx_.unlock();
                STRUCT2LOG_DEBUG_INFO(log.str());
            }
        }
    protected:
        std::string generate_file_name()
        {
            try
            {
                char task_name[STRUCT2LOG_PROCESS_NAEME_SIZE];
                char module_name[STRUCT2LOG_PROCESS_NAEME_SIZE];
                char task_fullname[STRUCT2LOG_PROCESS_NAEME_SIZE];
                pid_t pid = getpid();
                std::thread::id tid = std::this_thread::get_id();
                std::ostringstream tid_str;
                tid_str << tid;
                char proc_pid_path[STRUCT2LOG_BUF_SIZE];
                char buf[STRUCT2LOG_BUF_SIZE];
                //process name
                std::sprintf(proc_pid_path, "/proc/%d/status", pid);
                FILE* fp = std::fopen(proc_pid_path, "r");
                if(nullptr != fp)
                {
                    if(std::fgets(buf, STRUCT2LOG_BUF_SIZE-1, fp) == nullptr)
                    {
                        std::fclose(fp);
                        throw;
                    }
                    std::fclose(fp);
                    std::sscanf(buf, "%*s %s", task_name);
                   
                }
                //module name
                if(connection_.empty())
                {
                    std::sprintf(module_name, "%s", task_name);
                }
                else
                {
                    std::sprintf(module_name, "%s", connection_.c_str());
                }

                char date[STRUCT2LOG_DATE_BUF_SIZE];
                time_t now;
                time(&now);
                struct tm *tm_now;
                tm_now = localtime(&now);
                strftime(date, STRUCT2LOG_DATE_BUF_SIZE, "%F-%H-%M-%S", tm_now);

                char* env = getenv(STRUCT2LOG_EXT_LOG_PATH_PATTERN);
                if(nullptr != env)
                {
                    int str_statistics = 0;
                    int chr_statistics = 0;

                    char *tmp = env;
                    while(true)
                    {
                        tmp = strstr(tmp, "%s");
                        if(tmp != nullptr)
                        {
                            str_statistics++;
                            tmp+=2;
                        }
                        else
                        {
                            break;
                        }
                    }
                    tmp = env;
                    while(true)
                    {
                        tmp = strchr(tmp, '%');
                        if(tmp != nullptr)
                        {
                            chr_statistics++;
                            tmp+=1;
                        }
                        else
                        {
                            break;
                        }
                    }
                    if((str_statistics == chr_statistics) && (str_statistics > 0) && (str_statistics < 5))
                    {
                        std::sprintf(task_fullname, env, module_name, task_name, date, tid_str.str().c_str());
                        return std::string(task_fullname);
                    }
                }
                std::sprintf(task_fullname, STRUCT2LOG_LOG_FILE, module_name, task_name, date, tid_str.str().c_str());
                return std::string(task_fullname);
            }
            catch(std::exception& e)
            {
                STRUCT2LOG_DEBUG_EXCEPTION(e);
                return std::string("default.mdblog");
            }
        }
    private:
        std::ofstream file_;
        std::mutex mtx_;
    };



    //Field Type Checker Helper
    template <typename T>
    struct type_string
    {
        constexpr static const char *type =
            (std::is_same<T, std::uint8_t>::value)
            ? "tinyint unsigned "
            :(std::is_same<T, std::int8_t>::value)
            ? "tinyint "
            :(std::is_same<T, std::uint16_t>::value)
            ? "smallint unsigned "
            :(std::is_same<T, std::int16_t>::value)
            ? "smallint "
            :(std::is_same<T, std::uint32_t>::value)
            ? "int unsigned "
            :(std::is_same<T, std::int32_t>::value)
            ? "int "
            :(std::is_same<T, std::uint64_t>::value)
            ? "bigint unsigned "
            :(std::is_same<T, std::int64_t>::value)
            ? "bigint "
            :(std::is_floating_point<T>::value)
            ? "double "
            :(std::is_pointer<T>::value)
            ? "bigint unsigned "
            : " text ";;
    };

    // Injection Helper
    class injection_helper
    {
        static std::vector<std::string> &extract_name_token (std::vector<std::string> &vec, std::string input)
        {
            std::string tmp;

            for (const auto &ch : std::move (input) + ",")
            {
                if (isalnum (ch) || ch == '_')
                {
                    tmp += ch;
                }
                else if (ch == ',')
                {
                    if (!tmp.empty ()) vec.push_back (tmp);
                    tmp.clear ();
                }
            }
            return vec;
        };
        static std::string generate_value_string (std::vector<std::string> &vec)
        {
            std::ostringstream os;
            long count = vec.size();
            for(int i = 0; i < count; i++)
            {
                if(count == (i+1))
                {
                    os << vec[i];
                }
                else
                {
                    os << vec[i] << ",";
                }
            }
            return os.str();
        };

    public:
        //Value
        template <typename T, typename... Args>
        static inline std::vector<std::string> &serialize_value (std::vector<std::string> &vec, const T value, const Args... args)
        {
            serialize_value_ss(vec, value);
            return serialize_value(vec, args...);
        }
        template <typename T>
        static inline std::vector<std::string> &serialize_value (std::vector<std::string> &vec, const T value)
        {
            serialize_value_ss(vec, value);
            return vec;
        }

        
        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, const char *value)
        {
            std::ostringstream os;
            os << "\'" << value << "\'";
            vec.push_back(os.str());
            return vec;
        }
        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, const unsigned char *value)
        {
            std::ostringstream os;
            os << "\'" << value << "\'";
            vec.push_back(os.str());
            return vec;
        }

        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, char value)
        {
            vec.push_back(std::to_string((long)value));
            return vec;
        }
        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, unsigned char value)
        {
            vec.push_back(std::to_string((unsigned long)value));
            return vec;
        }

        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, short value)
        {
            vec.push_back(std::to_string((long)value));
            return vec;
        }
        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, unsigned short value)
        {
            vec.push_back(std::to_string((unsigned long)value));
            return vec;
        }
        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, int value)
        {
            vec.push_back(std::to_string(value));
            return vec;
        }
        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, unsigned int value)
        {
            vec.push_back(std::to_string(value));
            return vec;
        }
        
        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, long value)
        {
            vec.push_back(std::to_string(value));
            return vec;
        }
        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, unsigned long value)
        {
            vec.push_back(std::to_string(value));
            return vec;
        }

        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, long long value)
        {
            vec.push_back(std::to_string(value));
            return vec;
        }
        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, unsigned long long value)
        {
            vec.push_back(std::to_string(value));
            return vec;
        }
        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, float value)
        {
            vec.push_back(std::to_string(value));
            return vec;
        }
        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, double value)
        {
            vec.push_back(std::to_string(value));
            return vec;
        }
        //Type
        template <typename T>
        static inline std::vector<std::string> &serialize_type (std::vector<std::string> &vec, const T &)
        {
            vec.push_back (type_string<T>::type);
            return vec;
        }

        template <typename T, typename... Args>
        static inline std::vector<std::string> &serialize_type (std::vector<std::string> &vec, const T &, const Args&... args)
        {
            vec.push_back (type_string<T>::type);
            return serialize_type(vec, args...);
        }


    public:

        // Table Name Proxy
        template <typename C>
        static inline std::string table_name (const C &)
        {
            std::string table_name { C::__table_name };
            return table_name;
        }

        // Field Name Proxy
        template <typename C>
        static inline std::vector<std::string> field_names (const C &)
        {
            std::vector<std::string> field_names;
            extract_name_token (field_names, C::__field_names);
            return field_names;
        }
        // Field Type Proxy
        template <typename C>
        static inline std::vector<std::string> field_types (const C &obj)
        {
            std::vector<std::string> field_types;
            obj.__types(field_types);
            return field_types;
        }
        // Field Value Proxy
        template <typename C>
        static inline std::vector<std::string> field_values (const C &obj)
        {
            std::vector<std::string> field_values;
            obj.__values(field_values);
            return field_values;
        }
        // Field Value Proxy
        template <typename C>
        static inline std::string field_values_ss (const C &obj)
        {
            std::vector<std::string> field_values;
            obj.__values(field_values);
            return generate_value_string(field_values);
        }

        // Key Table Name Proxy
        template <typename C>
        static inline std::string field_pk_name (const C &)
        {
            std::string key_field_name { C::__key_field_name };
            return key_field_name;
        }

        // Key Field Type Proxy
        template <typename C>
        static inline std::string field_pk_type (const C &obj)
        {
            std::vector<std::string> vec;
            obj.__pk_type(vec);
            return vec[0];
        }
        // Key Field value Proxy
        template <typename C>
        static inline std::string field_pk_value (const C &obj)
        {
            std::vector<std::string> vec;
            obj.__pk_value(vec);
            return vec[0];
        }

    };
}


namespace struct2log_interface
{

    //struct2log
    class struct2log
    {

    public:
        struct2log ()
            :output_(true),handle_(nullptr)
        {
            init();
            return;
        }
        struct2log (std::string name)
            :name_(name),output_(true),handle_(nullptr)
        {
            init();
            return;
        }
        struct2log (bool output)
            :output_(output),handle_(nullptr)
        {
            init();
            return;
        }
        struct2log (std::string name, bool output)
            :name_(name),output_(output),handle_(nullptr)
        {
            init();
            return;
        }

        ~struct2log ()
        {
            fini();
            return;
        }
        void enable(bool enable)
        {
            if(output_ && !enable)
            {
                fini();
                output_ = enable;
            }
            else if(!output_ && enable)
            {
                output_ = enable;
                init();
            }
            return;
        }

        template <typename C>
        int create_tbl (const C &entity)
        {
            try
            {
                if(output_)
                {

                    auto table_name = struct2log_impl::injection_helper::table_name (entity);
                    auto field_names = struct2log_impl::injection_helper::field_names (entity);
                    auto field_types = struct2log_impl::injection_helper::field_types (entity);
                    auto field_pk_name = struct2log_impl::injection_helper::field_pk_name (entity);
                    auto field_pk_type = struct2log_impl::injection_helper::field_pk_type (entity);
                    //Generate log
                    std::ostringstream log;
                    log << "create table " << table_name << "(" << field_pk_name << "primary key not null,";
                    long count = field_names.size();
                    count = field_names.size();
                    for (int i = 0; i < count;i++)
                    {
                        if(count == (i+1))
                        {
                            log << field_names[i] << field_types[i] << ");" << std::endl;
                        }
                        else
                        {
                            log << field_names[i] << field_types[i] << ",";
                        }
                    }
                    //exec
                    connector_->execute (log);
                }
                return 0;
            }
            catch (std::exception& e)
            {
                STRUCT2LOG_DEBUG_EXCEPTION(e);
                return -1;
            }
        }

        template <typename C>
        int insert_row (const C &entity)
        {
            try
            {
                if(output_)
                {

                    auto table_name = struct2log_impl::injection_helper::table_name (entity);
                    auto field_values_ss = struct2log_impl::injection_helper::field_values_ss (entity);
                    auto field_pk_value = struct2log_impl::injection_helper::field_pk_value (entity);
                    //Generate log
                    std::ostringstream log;
                    log << "replace into " << table_name << " values (" << field_pk_value << "," << field_values_ss << ");" << std::endl;
                    //exec
                    connector_->execute (log);
                }
                return 0;
            }
            catch (std::exception& e)
            {
                STRUCT2LOG_DEBUG_EXCEPTION(e);
                return -1;
            }
        }

        template <typename C>
        int update_row (const C &entity)
        {
            try
            {
                if(output_)
                {

                    auto table_name = struct2log_impl::injection_helper::table_name (entity);
                    auto field_values_ss = struct2log_impl::injection_helper::field_values_ss (entity);
                    auto field_pk_value = struct2log_impl::injection_helper::field_pk_value (entity);
                    //Generate log
                    std::ostringstream log;
                    log << "replace into " << table_name << " values (" << field_pk_value << "," << field_values_ss << ");" << std::endl;
                    //exec
                    connector_->execute (log);
                }
                return 0;
            }
            catch (std::exception& e)
            {
                STRUCT2LOG_DEBUG_EXCEPTION(e);
                return -1;
            }
        }

        template <typename C>
        int delete_row (const C &entity)
        {
            try
            {
                if(output_)
                {

                    auto table_name = struct2log_impl::injection_helper::table_name (entity);
                    auto field_pk_name = struct2log_impl::injection_helper::field_pk_name (entity);
                    auto field_pk_value = struct2log_impl::injection_helper::field_pk_value (entity);
                    //Generate log
                    std::ostringstream log;
                    log << "delete from " << table_name << " where " << field_pk_name << "=" << field_pk_value << ";" << std::endl;
                    //exec
                    connector_->execute (log);
                }
                return 0;
            }
            catch (std::exception& e)
            {
                STRUCT2LOG_DEBUG_EXCEPTION(e);
                return -1;
            }
        }
    private:
        void init()
        {
            try
            {
                if(output_)
                {

                    char* env = nullptr;
                    env = getenv(STRUCT2LOG_EXT_CONNECTOR);
                    if(nullptr != env)
                    {
                        dlerror();
                        handle_ = dlopen(env, RTLD_NOW | RTLD_GLOBAL);
                        if(nullptr != handle_)
                        {
                            struct2log_impl::struct2log_connector* (*create_connector)(std::string&);
                            create_connector = (struct2log_impl::struct2log_connector*(*)(std::string&))dlsym(handle_, STRUCT2LOG_EXT_CONNECTOR_INIT);
                            if(nullptr != create_connector)
                            {
                                connector_.reset(create_connector(name_));
                                return;
                            }
                            else
                            {
                                STRUCT2LOG_DEBUG_ERROR(dlerror());
                            }
                        }

                    }
                    connector_.reset(new struct2log_impl::file_connector(name_));
                }
                return;
            }
            catch (std::exception& e)
            {
                STRUCT2LOG_DEBUG_EXCEPTION(e);
                return;
            }
        }
        void fini()
        {
            try
            {
                if(output_)
                {
                    connector_.reset();
                    if(nullptr != handle_)
                    {
                        dlclose(handle_);
                    }
                }
                return;
            }
            catch (std::exception& e)
            {
                STRUCT2LOG_DEBUG_EXCEPTION(e);
                return;
            }

        }
    private:
        std::string name_;
        bool output_;
        std::unique_ptr<struct2log_impl::struct2log_connector> connector_;
        void *handle_;
    };

}

#endif // _STRUCT2LOG_H_
