#include "ddg/http/http_parser.h"

#include <memory>

#include "ddg/config.h"
#include "ddg/http/http.h"
#include "ddg/log.h"

namespace ddg {

static Logger::ptr g_logger = DDG_LOG_ROOT();

namespace http {
static ddg::ConfigVar<uint64_t>::ptr g_http_request_buffer_size =
    ddg::Config::Lookup("http.request.buffer_size", (uint64_t)(4 * 1024),
                        "http request buffer size");

static ddg::ConfigVar<uint64_t>::ptr g_http_request_max_body_size =
    ddg::Config::Lookup("http.request.max_body_size",
                        (uint64_t)(64 * 1024 * 1024),
                        "http request max body size");

static ddg::ConfigVar<uint64_t>::ptr g_http_response_buffer_size =
    ddg::Config::Lookup("http.response.buffer_size", (uint64_t)(4 * 1024),
                        "http response buffer size");

static ddg::ConfigVar<uint64_t>::ptr g_http_response_max_body_size =
    ddg::Config::Lookup("http.response.max_body_size",
                        (uint64_t)(64 * 1024 * 1024),
                        "http response max body size");

static uint64_t s_http_request_buffer_size = 0;

static uint64_t s_http_request_max_body_size = 0;

static uint64_t s_http_response_buffer_size = 0;

static uint64_t s_http_response_max_body_size = 0;

namespace {
struct _RequestSizeIniter {
  _RequestSizeIniter() {
    s_http_request_buffer_size = g_http_request_buffer_size->getValue();
    s_http_request_max_body_size = g_http_request_max_body_size->getValue();
    s_http_response_buffer_size = g_http_response_buffer_size->getValue();
    s_http_response_max_body_size = g_http_response_max_body_size->getValue();

    g_http_request_buffer_size->addListener(
        [](const uint64_t& ov, const uint64_t& nv) {
          s_http_request_buffer_size = nv;
        });

    g_http_request_max_body_size->addListener(
        [](const uint64_t& ov, const uint64_t& nv) {
          s_http_request_max_body_size = nv;
        });

    g_http_response_buffer_size->addListener(
        [](const uint64_t& ov, const uint64_t& nv) {
          s_http_response_buffer_size = nv;
        });

    g_http_response_max_body_size->addListener(
        [](const uint64_t& ov, const uint64_t& nv) {
          s_http_response_max_body_size = nv;
        });
  }
};

static _RequestSizeIniter _init;
}  // namespace

uint64_t HttpRequestParser::GetHttpRequestBufferSize() {
  return s_http_request_buffer_size;
}

uint64_t HttpRequestParser::GetHttpRequestMaxBodySize() {
  return s_http_request_max_body_size;
}

uint64_t HttpResponseParser::GetHttpResponseBufferSize() {
  return s_http_response_buffer_size;
}

uint64_t HttpResponseParser::GetHttpResponseMaxBodySize() {
  return s_http_response_max_body_size;
}

void on_request_method(void* data, const char* at, size_t length) {
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
  HttpMethod m = CharsToHttpMethod(at);

  if (m == HttpMethod::INVALID_METHOD) {
    DDG_LOG_WARN(g_logger) << "invalid http request method: "
                           << std::string(at, length);
    return;
  }

  parser->getData()->setMethod(m);
}

void on_request_uri(void* data, const char* at, size_t length) {
  DDG_LOG_DEBUG(g_logger) << "on_request_uri";
}

void on_request_fragment(void* data, const char* at, size_t length) {
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
  parser->getData()->setFragment(std::string(at, length));
}

void on_request_path(void* data, const char* at, size_t length) {
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
  parser->getData()->setPath(std::string(at, length));
}

void on_request_query(void* data, const char* at, size_t length) {
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
  parser->getData()->setQuery(std::string(at, length));
}

void on_request_version(void* data, const char* at, size_t length) {
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
  uint8_t v = 0;
  if (strncmp(at, "HTTP/1.1", length) == 0) {
    v = 0x11;
  } else if (strncmp(at, "HTTP/1.1", length) == 0) {
    v = 0x00;
  } else {
    DDG_LOG_WARN(g_logger) << "invalid http request request version: "
                           << std::string(at, length);
    parser->setError(1001);
    return;
  }

  parser->getData()->setVersion(v);
}

void on_request_header_done(void* data, const char* at, size_t length) {
  DDG_LOG_DEBUG(g_logger) << "on_request_header_done";
}

void on_request_http_field(void* data, const char* field, size_t flen,
                           const char* value, size_t vlen) {
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
  if (flen == 0) {
    // setError(1002) P74:22:33 数据解析失败问题
    // 如果1002，如果header产生错误，就直接返回null
    // 虽然不把它当做错误，可能会有数据的错落，但是当成错误，更有问题
    DDG_LOG_WARN(g_logger) << "invalid http request field length == 0";
    return;
  }
  parser->getData()->setHeader(std::string(field, flen),
                               std::string(value, vlen));
}

void on_response_reason(void* data, const char* at, size_t length) {
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
  parser->getData()->setReason(std::string(at, length));
}

void on_response_status(void* data, const char* at, size_t length) {
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
  HttpStatus status = static_cast<HttpStatus>(atoi(at));
  parser->getData()->setStatus(status);
}

void on_response_chunk(void* data, const char* at, size_t length) {
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
  if (!parser->getData()->isChunk()) {
    parser->getData()->setChunk(true);
  }
  DDG_LOG_DEBUG(g_logger) << "on_response_chunk";
}

void on_response_version(void* data, const char* at, size_t length) {
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
  uint8_t v = 0;
  if (strncmp(at, "HTTP/1.1", length) == 0) {
    v = 0x11;
  } else if (strncmp(at, "HTTP/1.0", length) == 0) {
    v = 0x10;
  } else {
    DDG_LOG_WARN(g_logger) << "invalid http response version: "
                           << std::string(at, length);
    parser->setError(1001);
    return;
  }
  parser->getData()->setVersion(v);
}

void on_response_header_done(void* data, const char* at, size_t length) {
  DDG_LOG_DEBUG(g_logger) << "on_response_header_done";
}

void on_response_last_chunk(void* data, const char* at, size_t length) {
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
  if (parser->getData()->isChunk()) {
    parser->getData()->setChunk(true);
  }
  DDG_LOG_DEBUG(g_logger) << "on_response_last_chunk";
}

void on_response_http_field(void* data, const char* field, size_t flen,
                            const char* value, size_t vlen) {
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
  if (flen == 0) {
    DDG_LOG_WARN(g_logger) << "invalid http response field length == 0";
    return;
  }
  parser->getData()->setHeader(std::string(field, flen),
                               std::string(value, vlen));
}

HttpRequestParser::HttpRequestParser() : m_error(0) {
  m_data.reset(new ddg::http::HttpRequest);
  http_parser_init(&m_parser);
  m_parser.request_method = on_request_method;
  m_parser.request_uri = on_request_uri;
  m_parser.fragment = on_request_fragment;
  m_parser.request_path = on_request_path;
  m_parser.query_string = on_request_query;
  m_parser.http_version = on_request_version;
  m_parser.header_done = on_request_header_done;
  m_parser.http_field = on_request_http_field;
  m_parser.data = this;
}

size_t HttpRequestParser::execute(char* data, size_t len, bool withbody) {
  size_t offset = http_parser_execute(&m_parser, data, len, 0);

  if (withbody && isFinished()) {
    size_t content_length = getContentLength();

    if (content_length > static_cast<size_t>(len - offset)) {
      DDG_LOG_WARN(g_logger)
          << "HttpRequestParser::execute has invalid item(content_length)";
      content_length = len - offset;
    }

    m_data->setBody(std::string(data + offset, content_length));
    offset = offset + content_length;
  }

  memmove(data, data + offset, len - offset);

  return offset;
}

bool HttpRequestParser::isFinished() {
  if (m_error) {
    return false;
  }
  return http_parser_is_finished(&m_parser);
}

int HttpRequestParser::getError() {
  if (m_error) {
    return m_error;
  }
  m_error = http_parser_has_error(const_cast<http_parser*>(&m_parser));
  return m_error;
}

void HttpRequestParser::setError(int v) {
  m_error = v;
}

uint64_t HttpRequestParser::getContentLength() {
  return m_data->getHeaderAs<uint64_t>("content-length", 0);
}

HttpRequest::ptr HttpRequestParser::getData() const {
  return m_data;
}

const http_parser& HttpRequestParser::getParser() const {
  return m_parser;
}

HttpResponseParser::HttpResponseParser() : m_error(0) {
  m_data.reset(new ddg::http::HttpResponse);
  httpclient_parser_init(&m_parser);
  m_parser.reason_phrase = on_response_reason;
  m_parser.status_code = on_response_status;
  m_parser.chunk_size = on_response_chunk;
  m_parser.http_version = on_response_version;
  m_parser.header_done = on_response_header_done;
  m_parser.last_chunk = on_response_last_chunk;
  m_parser.http_field = on_response_http_field;
  m_parser.data = this;
}

size_t HttpResponseParser::execute(char* data, size_t len, bool chunk,
                                   bool withbody) {
  if (chunk) {
    httpclient_parser_init(&m_parser);
  }

  size_t offset = httpclient_parser_execute(&m_parser, data, len, 0);

  // TODO: chunk和content-length 是互斥的，不能互相存在
  // 参考网址：https://juejin.cn/post/7133865158304071694
  if (withbody && isFinished() && !chunk) {
    size_t content_length = getContentLength();

    if (content_length > static_cast<size_t>(len - offset)) {
      DDG_LOG_DEBUG(g_logger)
          << "HttpResponseParser::execute resize content_length from "
          << content_length << " to " << len - offset;
      content_length = len - offset;
    }

    m_data->setBody(std::string(data + offset, len - offset));
    offset = offset + content_length;
  }

  memmove(data, data + offset, len - offset);

  return offset;
}

bool HttpResponseParser::isFinished() {
  if (m_error) {
    return false;
  }
  return httpclient_parser_is_finished(&m_parser);
}

int HttpResponseParser::getError() {
  if (m_error) {
    return false;
  }
  int ret = httpclient_parser_has_error(&m_parser);
  m_error = ret;
  return ret;
}

uint64_t HttpResponseParser::getContentLength() {
  return m_data->getHeaderAs<uint64_t>("content-length", 0);
}

}  // namespace http

}  // namespace ddg
