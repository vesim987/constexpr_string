#include <cstddef>
#include <utility>


#if defined(_MSC_VER)
#   if _MSC_VER < 1800
#       error This library needs at least Visual Studio 2013
#   endif
#elif __cplusplus < 201103L
#	error This library needs at least a C++11 compiler
#endif

//std::index_sequence for C++11
#if __cplusplus == 201103L
namespace std {
    //author: http://stackoverflow.com/a/32223343
    template <size_t... Ints>
    struct index_sequence {
        using type = index_sequence;
        using value_type = size_t;
        static constexpr std::size_t size()
        {
            return sizeof...(Ints);
        }
    };

    // --------------------------------------------------------------

    template <class Sequence1, class Sequence2>
    struct _merge_and_renumber;

    template <size_t... I1, size_t... I2>
    struct _merge_and_renumber<index_sequence<I1...>, index_sequence<I2...>>
            : index_sequence<I1..., (sizeof...(I1)+I2)...> {
              };

    // --------------------------------------------------------------

    template <size_t N>
    struct make_index_sequence
        : _merge_and_renumber<typename make_index_sequence<N / 2>::type,
          typename make_index_sequence<N - N / 2>::type> {
    };

    template<> struct make_index_sequence<0> : index_sequence<> { };
    template<> struct make_index_sequence<1> : index_sequence<0> { };
}
#endif

namespace constexpr_string {
    template<std::size_t Length>
    struct string;


    template<std::size_t N>
    constexpr const string<N> make_string(const char(&str)[N]);

    template<std::size_t Length>
    struct string {
        const char value[Length];
        const std::size_t length;

        #pragma region ctors
        template<class... Chars>
        constexpr string(const char c, const Chars... chars) : value{ c, chars... },
            length(Length) { }

        template<std::size_t... Idx>
        constexpr string(const char str[Length], std::index_sequence<Idx...>) : value{ str[Idx]... },
            length(Length) { }
        #pragma endregion

        #pragma region getters
        constexpr const char* operator() () const
        {
            return data();
        }

        constexpr const char* data() const
        {
            return value;
        }

        #pragma endregion

        #pragma region concatenate
        template<std::size_t Other_length>
        constexpr string<Other_length + Length - 1/* one null terminator*/>
        operator + (const string<Other_length> other) const
        {
            return concatenate_helper(other,
                                      std::make_index_sequence<Length - 1> {},
                                      std::make_index_sequence<Other_length> {});
        }
    private:
        template<std::size_t Other_length, std::size_t... Idx, std::size_t... Idx_other>
        constexpr string<Other_length + Length - 1> concatenate_helper(
            const string<Other_length> other,
            std::index_sequence<Idx...>,
            std::index_sequence<Idx_other...>) const
        {
            return { value[Idx]..., other.value[Idx_other]... };
        }
        #pragma endregion

        #pragma region substr
    public:
        template<std::size_t Pos, std::size_t Len>
        constexpr string<Len + 1> substr() const
        {
            return substr_helper<Pos, Len>(std::make_index_sequence<Len> {});
        }
    private:
        template<std::size_t Pos, std::size_t Len, std::size_t... Idx>
        constexpr string<Len + 1> substr_helper(std::index_sequence<Idx...>) const
        {
            return { value[Pos + Idx]..., '\0' };
        }
        #pragma endregion

        #pragma region to_lower
    public:
        constexpr string<Length> to_lower() const
        {
            return to_lower_helper(std::make_index_sequence<Length> {});
        }
    private:
        template<std::size_t... Idx>
        constexpr string<Length> to_lower_helper(std::index_sequence<Idx...>) const
        {
            return { to_lower_char(value[Idx])... };
        }
        constexpr char to_lower_char(const char c) const
        {
            return (c >= 'A' && c <= 'Z') ? c + ' ' : c;
        }
        #pragma endregion

        #pragma region to_upper
    public:
        constexpr string<Length> to_upper() const
        {
            return to_upper_helper(std::make_index_sequence<Length> {});
        }
    private:
        template<std::size_t... Idx>
        constexpr string<Length> to_upper_helper(std::index_sequence<Idx...>) const
        {
            return { to_upper_char(value[Idx])... };
        }
        constexpr char to_upper_char(const char c) const
        {
            return (c >= 'a' && c <= 'z') ? c - ' ' : c;
        }
        #pragma endregion

        #pragma region find
    public:
        constexpr std::size_t find(const char c, const std::size_t i = 0) const
        {
            return i >= Length ? 0xFFFFFFFF : value[i] == c ? i : find(c, i + 1);
        }

        template<std::size_t Str_length>
        constexpr std::size_t find(const string<Str_length> str,
                                   const std::size_t i = 0) const
        {
            return i >= Length ? 0xFFFFFFFF :
                   string_check(i, 0, str) ? i : find(str, i + 1);
        }

    private:
        template<std::size_t Str_length>
        constexpr bool string_check(const std::size_t i, const std::size_t i2,
                                    const string<Str_length> str) const
        {
            return i2 >= Str_length ? false :
                   value[i] == str.value[i2] ? true : string_check(i + 1, i2 + 1, str);
        }
        #pragma endregion

        #pragma region rfind
    public:
        constexpr std::size_t rfind(const char c,
                                    const std::size_t i = Length - 1) const
        {
            return i == 0 ?
                   value[i] == c ? i : 0xFFFFFFFF :
                   value[i] == c ? i : rfind(c, i - 1);
        }

        template<std::size_t Str_length>
        constexpr std::size_t rfind(const string<Str_length> str,
                                    const std::size_t i = Length - Str_length) const
        {
            return i == 0 ?
                   string_check(i, 0, str) ? i : 0xFFFFFFFF :
                   string_check(i, 0, str) ? i : rfind(str, i - 1);
        }
        #pragma endregion

        #pragma region replace
    public:
        constexpr string<Length> replace(const char from, const char to) const
        {
            return replace_helper(from, to, std::make_index_sequence<Length> {});
        }
    private:
        template<std::size_t... Idx>
        constexpr string<Length> replace_helper(const char from, const char to,
                                                std::index_sequence<Idx...>) const
        {
            return { replace_char(value[Idx], from, to)... };
        }
        constexpr char replace_char(char c, const char from, const char to) const
        {
            return c == from ? to : c;
        }
        #pragma endregion


    };


    template<std::size_t N>
    constexpr const string<N> make_string(const char(&str)[N])
    {
        return { str, std::make_index_sequence<N>{} };
    }

};