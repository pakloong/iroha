/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_POSTGRES_BLOCK_INDEX_HPP
#define IROHA_POSTGRES_BLOCK_INDEX_HPP

#include "ametsuchi/impl/block_index.hpp"
#include "ametsuchi/impl/postgres_wsv_common.hpp"
#include "logger/logger.hpp"

#include <boost/format.hpp>
#include <interfaces/transaction.hpp>
#include <pqxx/nontransaction>

namespace shared_model {
  namespace interface {
    class Command;
  }
}  // namespace shared_model
namespace iroha {
  namespace ametsuchi {
    class PostgresBlockIndex : public BlockIndex {
      /// Type of ordered collection of commands
      using CommandsType = shared_model::interface::Transaction::CommandsType;

     public:
      explicit PostgresBlockIndex(pqxx::nontransaction &transaction);

      void index(const w<shared_model::interface::Block> block) override;

     private:
      /**
       * Make index account_id -> list of blocks where his txs exist
       * @param account_id of transaction creator
       * @param height of block
       */
      auto indexAccountIdHeight(const std::string &account_id,
                                const std::string &height);

      /**
       * Collect all assets belonging to creator, sender, and receiver
       * to make account_id:height:asset_id -> list of tx indexes (where
       * tx with certain asset is placed in the block)
       * @param account_id of transaction creator
       * @param height of block
       * @param index of transaction in the block
       * @param commands in the transaction
       */
      auto indexAccountAssets(const std::string &account_id,
                              const std::string &height,
                              const std::string &index,
                              const CommandsType &commands);

      pqxx::nontransaction &transaction_;
      logger::Logger log_;
      using ExecuteType = decltype(makeExecute(transaction_, log_));
      ExecuteType execute_;

      // TODO: refactor to return Result when it is introduced IR-775
      bool execute(const std::string &statement) noexcept {
        return static_cast<bool>(execute_(statement));
      }
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_BLOCK_INDEX_HPP
