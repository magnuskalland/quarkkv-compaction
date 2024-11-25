#pragma once

#include "core/core_workload.h"
#include "core/workload_proxy.h"

/**
 * This code is inspired by and adapted from
 * https://github.com/SpanDB/SpanDB/blob/master/ycsb/src/workloadwrapper.h
 */
namespace ycsbc {
class WorkloadProducer {
   public:
    struct Request {
       private:
        Operation opt_;
        const std::string key_;
        int len_;

       public:
        Request(Operation opt, std::string key) : opt_(opt), key_(key)
        {
            len_ = 0;
        }
        Request(Operation opt, std::string key, int len) : opt_(opt), key_(key), len_(len)
        {
        }

        Operation Type()
        {
            return opt_;
        }

        std::string Key()
        {
            return key_;
        }

        int Len()
        {
            return len_;
        }

        void SetType(Operation opt)
        {
            opt_ = opt;
        }
    };

    WorkloadProducer(WorkloadProxy *proxy) : proxy_(proxy) {}

    Request *GetRequest()
    {
        Operation opt;
        std::string table, key;
        std::vector<std::string> fields;
        std::vector<ycsbc::CoreWorkload::KVPair> values;
        int len;

        while (true) {
            opt = proxy_->GetNextOperation();
            switch (opt) {
                case READ:
                    proxy_->GetReadArgs(table, key, fields);
                    return new Request(READ, key);
                case UPDATE:
                    proxy_->GetUpdateArgs(table, key, values);
                    assert(values.size() > 0);
                    return new Request(UPDATE, key);
                case INSERT:
                    proxy_->GetInsertArgs(table, key, values);
                    assert(values.size() > 0);
                    return new Request(UPDATE, key);
                case READMODIFYWRITE:
                    proxy_->GetReadModifyWriteArgs(table, key, fields, values);
                    assert(values.size() > 0);
                    return new Request(READMODIFYWRITE, key);
                case SCAN:
                    proxy_->GetScanArgs(table, key, len, fields);
                    return new Request(SCAN, key, len);
                default:
                    continue;
            }
        }
        return nullptr;
    }

    Request *GetLoadRequest()
    {
        std::string table, key;
        std::vector<ycsbc::CoreWorkload::KVPair> values;
        proxy_->GetInsertArgs(table, key, values);
        assert(values.size() > 0);
        return new Request(INSERT, key);
    }

   private:
    WorkloadProxy *proxy_ = nullptr;
};

}  // namespace ycsbc