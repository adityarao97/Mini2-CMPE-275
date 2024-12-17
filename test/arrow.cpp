#include "arrow/csv/api.h"
#include "arrow/io/file.h"
#include "arrow/dataset/api.h"
#include "arrow/array/array_primitive.h"
#include <iostream>
#include <memory>


int main() {
    std::string path = "../test/test-data.csv";
    arrow::io::IOContext io_context = arrow::io::default_io_context();
    auto readable = arrow::io::ReadableFile::Open(path);

    std::shared_ptr<arrow::io::InputStream> input = *readable;
    auto read_options = arrow::csv::ReadOptions::Defaults();
    auto parse_options = arrow::csv::ParseOptions::Defaults();
    auto convert_options = arrow::csv::ConvertOptions::Defaults();

    // Instantiate TableReader from input stream and options
    auto maybe_reader =
        arrow::csv::TableReader::Make(io_context,
                                    input,
                                    read_options,
                                    parse_options,
                                    convert_options);
    if (!maybe_reader.ok()) {
        // Handle TableReader instantiation error...
        std::cout << "TableReader error" << std::endl;
    }
    std::shared_ptr<arrow::csv::TableReader> reader = *maybe_reader;

    // Read table from CSV file
    auto maybe_table = reader->Read();
    if (!maybe_table.ok()) {
            std::cout << "error reading csv" << std::endl;
        // Handle CSV read error
        // (for example a CSV syntax error or failed type conversion)
    }
    std::shared_ptr<arrow::Table> table = *maybe_table;
    std::cout << table->schema()->ToString() << std::endl;

    // try to write simple query
    std::shared_ptr<arrow::dataset::Dataset> dataset = std::make_shared<arrow::dataset::InMemoryDataset>(table);

    // 2: Build ScannerOptions for a Scanner to do a basic filter operation
    auto options = std::make_shared<arrow::dataset::ScanOptions>();

    int a = 77050;
    options->filter = arrow::compute::greater(
        arrow::compute::field_ref("2003"), 
        arrow::compute::literal(1000000000)); // Change for your use case

    //options->projected_schema = 
    // 3: Build the Scanner
    auto builder = arrow::dataset::ScannerBuilder(dataset, options);
    auto scanner = builder.Finish();

    // 4: Perform the Scan and make a Table with the result
    auto result = scanner.ValueUnsafe()->ToTable();
    // need to loop through the result 

    auto arrow_int_array = std::static_pointer_cast<arrow::Int64Array>(result.ValueUnsafe()->GetColumnByName("1995")->chunk(0));
    std::vector<int64_t> result_vec; 
    for (int i = 0; i < arrow_int_array->length(); i++) {
        result_vec.push_back(arrow_int_array->Value(i));
    }

    for (auto &tmp: result_vec) {
        std::cout << tmp << " ";
    }
    std::cout << std::endl;
    std::cout << arrow_int_array->ToString() << std::endl;
}
