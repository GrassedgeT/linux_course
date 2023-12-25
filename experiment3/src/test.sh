#!/bin/bash

# 服务器的 URL
url="http://localhost:8080"

# 正确返回的请求次数
success_count=0

while true; do
    # 发起 GET 请求
    response=$(curl -s -o /dev/null -w "%{http_code}" $url)
    if [ "$response" -eq 200 ]; then
        ((success_count++))
    fi

    # 发起 POST 请求
    response=$(curl -s -o /dev/null -w "%{http_code}" -X POST $url)
    if [ "$response" -eq 400 ]; then
        ((success_count++))
    fi

    echo "Number of successful requests: $success_count"
done