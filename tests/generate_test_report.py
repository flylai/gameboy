import json
import sys


def convert_to_markdown(commit_id, json_file):
    repo_url = "https://github.com/flylai/gameboy/commit/"

    with open(json_file, 'r') as file:
        data = json.load(file)

    markdown_str = f"# Google Test Report\n\nCommit: [{commit_id}]({repo_url}{commit_id})\n\n"

    for suite in data['testsuites']:
        markdown_str += f"## Suite: {suite['name']}\n"
        for test in suite['testsuite']:
            # Determine the emoji based on the test result
            emoji = "✅" if "failures" not in test or not test["failures"] else "❌"
            markdown_str += f"- {emoji} **Test**: {test['value_param']}\n"

    return markdown_str


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: script.py <commit_id> <json_file_path>")
        sys.exit(1)

    commit_id = sys.argv[1]
    json_file_path = sys.argv[2]

    markdown_report = convert_to_markdown(commit_id, json_file_path)
    print(markdown_report)
