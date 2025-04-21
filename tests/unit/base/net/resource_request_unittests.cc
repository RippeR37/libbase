#if defined(LIBBASE_MODULE_NET)

#include "base/net/resource_request.h"

#include <string>

#include "gtest/gtest.h"

namespace {
const std::string kTestUrl = "http://example.com";
const std::string kTestHeader1 = "Key1: Value1";
const std::string kTestHeader2 = "Key2: Value2";
const std::vector<uint8_t> kTestPostData1 = {'T', 'e', 's', 't', '1'};
const std::vector<uint8_t> kTestPostData2 = {'T', 'e', 's', 't', '2'};
const std::string kTestPostDataStr1 = "Test1";
const std::string kTestPostDataStr2 = "Test2";
}  // namespace

base::net::ResourceRequest GetBaseRequest() {
  return base::net::ResourceRequest{kTestUrl};
}

TEST(NET_ResourceRequest, SimpleWithLvalue) {
  {
    auto req = GetBaseRequest();
    auto&& req2 = req.WithHeaders({kTestHeader1});

    EXPECT_TRUE(req.headers.empty());
    EXPECT_EQ(req2.headers.size(), 1u);
    EXPECT_EQ(req2.headers[0], kTestHeader1);
  }

  {
    auto req = GetBaseRequest();
    auto&& req2 = req.WithPostData(kTestPostData1);

    EXPECT_TRUE(!req.post_data);
    ASSERT_TRUE(req2.post_data);
    EXPECT_EQ(*req2.post_data, kTestPostData1);
  }

  {
    auto req = GetBaseRequest();
    auto&& req2 = req.WithPostData(kTestPostDataStr1);

    EXPECT_TRUE(!req.post_data);
    ASSERT_TRUE(req2.post_data);
    EXPECT_EQ(*req2.post_data, kTestPostData1);
  }

  {
    auto req = GetBaseRequest();
    const bool org_value = req.headers_only;
    auto&& req2 = req.WithHeadersOnly(!org_value);

    EXPECT_EQ(req.headers_only, org_value);
    EXPECT_EQ(req2.headers_only, !org_value);
  }

  {
    auto req = GetBaseRequest();
    auto&& req2 = req.WithTimeout(base::Seconds(10));

    EXPECT_TRUE(req.timeout.IsZero());
    EXPECT_EQ(req2.timeout, base::Seconds(10));
  }

  {
    auto req = GetBaseRequest();
    auto&& req2 = req.WithConnectTimeout(base::Seconds(20));

    EXPECT_TRUE(req.connect_timeout.IsZero());
    EXPECT_EQ(req2.connect_timeout, base::Seconds(20));
  }

  {
    auto req = GetBaseRequest();
    const bool org_value = req.follow_redirects;
    auto&& req2 = req.WithFollowRedirects(!org_value);

    EXPECT_EQ(req.follow_redirects, org_value);
    EXPECT_EQ(req2.follow_redirects, !org_value);
  }
}

TEST(NET_ResourceRequest, SimpleWithRvalue) {
  {
    auto req = GetBaseRequest().WithHeaders({kTestHeader1});

    ASSERT_EQ(req.headers.size(), 1u);
    EXPECT_EQ(req.headers[0], kTestHeader1);
  }

  {
    auto req = GetBaseRequest().WithPostData(kTestPostData1);
    ;

    ASSERT_TRUE(req.post_data);
    EXPECT_EQ(*req.post_data, kTestPostData1);
  }

  {
    auto req = GetBaseRequest().WithPostData(kTestPostDataStr1);

    ASSERT_TRUE(req.post_data);
    EXPECT_EQ(*req.post_data, kTestPostData1);
  }

  {
    auto req = GetBaseRequest();
    auto req2 = GetBaseRequest().WithHeadersOnly(!req.headers_only);

    EXPECT_NE(req.headers_only, req2.headers_only);
  }

  {
    auto req = GetBaseRequest().WithTimeout(base::Seconds(10));
    ;

    EXPECT_EQ(req.timeout, base::Seconds(10));
  }

  {
    auto req = GetBaseRequest().WithConnectTimeout(base::Seconds(20));

    EXPECT_EQ(req.connect_timeout, base::Seconds(20));
  }

  {
    auto req = GetBaseRequest();
    auto req2 = GetBaseRequest().WithFollowRedirects(!req.follow_redirects);

    EXPECT_NE(req.follow_redirects, req2.follow_redirects);
  }
}

#endif  // defined(LIBBASE_MODULE_NET)
