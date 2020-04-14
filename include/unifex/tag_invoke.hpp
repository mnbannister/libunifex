/*
 * Copyright 2019-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <unifex/config.hpp>
#include <unifex/detail/concept_macros.hpp>

#include <type_traits>

namespace unifex {
  namespace _tag_invoke {
    void tag_invoke();

    struct _fn {
      template <typename CPO, typename... Args>
      constexpr auto operator()(CPO cpo, Args&&... args) const
          noexcept(noexcept(tag_invoke((CPO &&) cpo, (Args &&) args...)))
          -> decltype(tag_invoke((CPO &&) cpo, (Args &&) args...)) {
        return tag_invoke((CPO &&) cpo, (Args &&) args...);
      }
    };

    template <typename CPO, typename... Args>
    using tag_invoke_result_t = decltype(tag_invoke(
        static_cast<CPO && (*)() noexcept>(nullptr)(),
        static_cast<Args && (*)() noexcept>(nullptr)()...));

    struct yes_type {
      char dummy;
    };
    struct no_type {
      char dummy[2];
    };

    template <typename CPO, typename... Args>
    auto try_tag_invoke(int) noexcept(noexcept(tag_invoke(
        static_cast<CPO && (*)() noexcept>(nullptr)(),
        static_cast<Args && (*)() noexcept>(nullptr)()...)))
        -> decltype(
            static_cast<void>(tag_invoke(
                static_cast<CPO && (*)() noexcept>(nullptr)(),
                static_cast<Args && (*)() noexcept>(nullptr)()...)),
            yes_type{});

    template <typename CPO, typename... Args>
    no_type try_tag_invoke(...) noexcept(false);

    template <template <typename...> class T, typename... Args>
    struct defer {
      using type = T<Args...>;
    };

    struct empty {};
  }  // namespace _tag_invoke

  namespace _tag_invoke_cpo {
    UNIFEX_INLINE_VAR constexpr _tag_invoke::_fn tag_invoke{};
  }
  using namespace _tag_invoke_cpo;

  template <auto& CPO>
  using tag_t = std::remove_cvref_t<decltype(CPO)>;

  // Manually implement the traits here rather than defining them in terms of
  // the corresponding std::invoke_result/is_invocable/is_nothrow_invocable
  // traits to improve compile-times. We don't need all of the generality of the
  // std:: traits and the tag_invoke traits are used heavily through libunifex
  // so optimising them for compile time makes a big difference.

  using _tag_invoke::tag_invoke_result_t;

  template <typename CPO, typename... Args>
  UNIFEX_INLINE_VAR constexpr bool is_tag_invocable_v =
      (sizeof(_tag_invoke::try_tag_invoke<CPO, Args...>(0)) ==
       sizeof(_tag_invoke::yes_type));

  template <typename CPO, typename... Args>
  struct tag_invoke_result
    : std::conditional_t<
          is_tag_invocable_v<CPO, Args...>,
          _tag_invoke::defer<tag_invoke_result_t, CPO, Args...>,
          _tag_invoke::empty> 
  {};

  template <typename CPO, typename... Args>
  struct is_tag_invocable
    : std::bool_constant<is_tag_invocable_v<CPO, Args...>>
  {};

  template <typename CPO, typename... Args>
  UNIFEX_INLINE_VAR constexpr bool is_nothrow_tag_invocable_v =
      noexcept(_tag_invoke::try_tag_invoke<CPO, Args...>(0));

  template <typename CPO, typename... Args>
  struct is_nothrow_tag_invocable
    : std::bool_constant<is_nothrow_tag_invocable_v<CPO, Args...>>
  {};

  template <typename CPO, typename... Args>
  UNIFEX_CONCEPT tag_invocable =
      (sizeof(_tag_invoke::try_tag_invoke<CPO, Args...>(0)) ==
       sizeof(_tag_invoke::yes_type));

  namespace lazy {
    template <typename CPO, typename... Args>
    UNIFEX_CONCEPT_DEFER tag_invocable =
        UNIFEX_DEFER_(unifex::tag_invocable, UNIFEX_TYPE(CPO), Args...);
  } // namespace lazy
}  // namespace unifex
