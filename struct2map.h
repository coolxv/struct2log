#ifndef _STRUCT2MAP_H_
#define _STRUCT2MAP_H_

//for C
#include <cstdint>
//for container
#include <tuple>
#include <vector>
#include <list>
#include <string>
#include <map>
//for serialization
#include <sstream>
//for type traits
#include <type_traits>

//for base type
#include <cinttypes>


//Public Macro
#define STRUCT2MAP(_TABLE_NAME_, ...)\
private:\
   constexpr static const char *__field_names = #__VA_ARGS__;\
private:\
    friend class struct2map_impl::injection_helper;\
private:\
    inline std::vector<std::string> &__values(std::vector<std::string> &vec)\
    { return struct2map_impl::injection_helper::serialize_value (vec, __VA_ARGS__); }\
    inline std::vector<std::string> &__values(std::vector<std::string> &vec) const\
    { return struct2map_impl::injection_helper::serialize_value (vec, __VA_ARGS__); }\




namespace struct2map_impl
{

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
            os << value ;
            vec.push_back(os.str());
            return vec;
        }
        static inline std::vector<std::string> &serialize_value_ss (std::vector<std::string> &vec, const unsigned char *value)
        {
            std::ostringstream os;
            os << value;
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


    public:

        // Field Name Proxy
        template <typename C>
        static inline std::vector<std::string> field_names (const C &)
        {
            std::vector<std::string> field_names;
            extract_name_token (field_names, C::__field_names);
            return field_names;
        }

        // Field Value Proxy
        template <typename C>
        static inline std::vector<std::string> field_values (const C &obj)
        {
            std::vector<std::string> field_values;
            obj.__values(field_values);
            return field_values;
        }

    };
}


namespace struct2map_interface
{

    //struct2map
    class struct2map
    {
    public:

        template <typename C>
        void to_map (const C &entity, std::map<std::string, std::string> &result, std::string (*transform)(std::string, std::string)= nullptr)
        {
            auto field_names = struct2map_impl::injection_helper::field_names (entity);
            auto field_values = struct2map_impl::injection_helper::field_values(entity);

            //Generate log
            long count = field_names.size();
            if (transform)
            {
                for (int i = 0; i < count;i++)
                {
                    result[field_names[i]] = transform(field_names[i], field_values[i]);
                }
            }
            else
            {
                for (int i = 0; i < count;i++)
                {
                    result[field_names[i]] = field_values[i];
                }
            }

        }





    };

}

#endif // _STRUCT2MAP_H_
