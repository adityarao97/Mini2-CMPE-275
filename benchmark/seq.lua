counter = 0

-- for response statistics

request = function()
    counter = counter + 1
    return wrk.format(nil, "/search_string?headerKey=location1&headerValue=%22Jacobs%22")
end

done = function(summary, latency, requests)
    io.write(string.format("Latency Distribution:\n"))
    io.write(string.format("  p50: %d ms\n", latency:percentile(50) / 1000))
    io.write(string.format("  p90: %d ms\n", latency:percentile(90) / 1000))
    io.write(string.format("  p99: %d ms\n", latency:percentile(99) / 1000))
end