//
// Created by Mehmet Fatih BAKIR on 27/04/2018.
//

#pragma once

#include <unordered_map>
#include <tvm/tvm_types.hpp>
#include <tvm/instr_traits.hpp>
#include <array>

namespace tvm {

    enum class operand_type
    {
        error,
        reg,
        literal
    };

    struct operand_description
    {
        operand_type type = operand_type::error;
        uint8_t bits = 0;
        uint8_t offset = 0;
    };

    template<uint8_t N>
    constexpr operand_description get_descr(ctype<reg_ind_t<N>>)
    {
        operand_description res;
        res.bits = N;
        res.type = operand_type::reg;
        return res;
    }

    template<uint8_t N>
    constexpr operand_description get_descr(ctype<operand_t<N>>)
    {
        operand_description res;
        res.bits = N;
        res.type = operand_type::literal;
        return res;
    }

    template<class... Ts>
    std::vector<operand_description> get_descrs(list<Ts...>)
    {
        return {get_descr(ctype<Ts>{})...};
    }

    class instr_data
    {
    public:
        virtual uint8_t get_opcode() const = 0;

        virtual const char* mnemonic() const = 0;

        virtual int32_t operand_count() const = 0;

        virtual uint8_t get_size() const = 0;

        virtual std::vector<operand_description>
        get_operands() const = 0;

        virtual std::vector<size_t>
        get_offsets() const = 0;

        virtual ~instr_data() = default;
    };

    template<class T, uint8_t opc>
    class concrete_instr_data : public instr_data
    {
    public:
        uint8_t get_opcode() const override
        {
            return opc;
        }

        const char* mnemonic() const override
        {
            return instr_name_v<T>;
        }

        uint8_t get_size() const override {
            return instruction_len<T>();
        }

        int32_t operand_count() const override
        {
            return tvm::operand_count<T>();
        }

        std::vector<operand_description> get_operands() const override
        {
            using traits = functor_traits<T>;
            using args = tail_t<typename traits::arg_ts>;
            return get_descrs(args{});
        }

        std::vector<size_t> get_offsets() const override {
            using traits = functor_traits<T>;
            using args = tail_t<typename traits::arg_ts>;
            using offsets_t = offsets<args>;
            auto arr = to_array(typename offsets_t::type{});
            return std::vector<size_t>(arr.begin(), arr.end());
        }
    };

    class isa_description
    {
        using instr_data_map = std::unordered_map<std::string, instr_data*>;
    public:
        explicit isa_description(instr_data_map&& map)
                : m_instrs{std::move(map)} {}

        const instr_data* get(const std::string& mnemonic) const
        {
            auto it = m_instrs.find(mnemonic);
            if (it == m_instrs.end())
            {
                return nullptr;
            }
            return it->second;
        }

        auto begin() const {
            return m_instrs.begin();
        }

        auto end() const
        {
            return m_instrs.end();
        }
    private:
        instr_data_map m_instrs;
    };

    template<uint8_t... opcodes, class... InstTs>
    auto describe(list<ins<opcodes, InstTs>...>)
    {
        std::unordered_map<std::string, instr_data*> res{
                {instr_name_v<InstTs>, new concrete_instr_data<InstTs, opcodes>{} }...
        };
        return isa_description{std::move(res)};
    }
}
