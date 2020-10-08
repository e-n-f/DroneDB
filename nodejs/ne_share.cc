/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include <iostream>
#include "shareservice.h"
#include "exceptions.h"
#include "ne_share.h"
#include "ne_helpers.h"

class ShareWorker : public Nan::AsyncProgressWorker {
 public:
  ShareWorker(Nan::Callback *callback, Nan::Callback *progress, const std::vector<std::string> &input, const std::string &tag, const std::string &password,
              bool recursive)
    : Nan::AsyncProgressWorker(callback, "nan:ShareWorker"),
      progress(progress),
      input(input), tag(tag), password(password),
      recursive(recursive), cancel(false) {}
  ~ShareWorker() {
      delete progress;
  }

  void Execute (const Nan::AsyncProgressWorker::ExecutionProgress& progress) {
    ddb::ShareService ss;
    try{
        ddb::ShareCallback showProgress = [&](const std::string &file, float prog){
            // Prepare JSON object
            json j = {
                "file", file,
                "progress", prog
            };
            std::string serialized = j.dump();
            progress.Send(serialized.c_str(), sizeof(char) * serialized.length());
            return !cancel;
        };

        url = ss.share(input, tag, password, recursive, "", showProgress);
    }catch(const ddb::AuthException &){
        SetErrorMessage("Unauthorized");
    }catch(const ddb::AppException &e){
        SetErrorMessage(e.what());
    }
  }

  void HandleProgressCallback(const char *data, size_t count) {
      Nan::HandleScope scope;
      Nan::JSON json;

      std::string str(data, count);

      v8::Local<v8::Value> argv[] = {
          json.Parse(Nan::New<v8::String>(str).ToLocalChecked()).ToLocalChecked()
      };

      cancel = !Nan::To<bool>(progress->Call(1, argv, async_resource).ToLocalChecked()).FromJust();
  }

  void HandleOKCallback () {
     Nan::HandleScope scope;

     v8::Local<v8::Value> argv[] = {
         Nan::Null(),
         Nan::New(url).ToLocalChecked()
     };
     callback->Call(2, argv, async_resource);
   }

 private:
    Nan::Callback *progress;
    std::vector<std::string> input;
    std::string tag;
    std::string password;
    bool recursive;

    bool cancel;

    std::string url;
};


NAN_METHOD(share) {
    ASSERT_NUM_PARAMS(4);

    BIND_STRING_ARRAY_PARAM(paths, 0);

    BIND_OBJECT_PARAM(obj, 1);
    BIND_OBJECT_STRING(obj, tag, "");
    BIND_OBJECT_STRING(obj, password, "");
    BIND_OBJECT_VAR(obj, bool, recursive, false);

    BIND_FUNCTION_PARAM(progress, 2);
    BIND_FUNCTION_PARAM(callback, 3);

    Nan::AsyncQueueWorker(new ShareWorker(callback, progress, paths, tag, password, recursive));
}
