#pragma once

#include <optional>
#include <string>
#include <queue>
#include <map>

class DirectoryWatcher {
public:
    enum class Status { Ready, Processing, FinishedSuccess, FinishedFailure };

private:
    std::string m_directory_to_watch;
    std::queue<std::string> m_ready_filenames;
    std::map<std::string, Status> m_directory_contents;

public:
    void SetDirectoryToWatch(std::string path) { m_directory_to_watch = path; }

    void Update();

    std::optional<std::string> GetNextFilename();

    void FinishSuccess(const std::string& filename);
    void FinishFailure(const std::string& filename);
};

