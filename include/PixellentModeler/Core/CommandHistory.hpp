#pragma once

#include "PixellentModeler/Core/Command.hpp"
#include <memory>
#include <vector>
#include <string>

namespace PixellentModeler {

class CommandHistory {
public:
    void execute(std::unique_ptr<Command> cmd);
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;
    std::string undoDescription() const;
    std::string redoDescription() const;
    void clear();

private:
    std::vector<std::unique_ptr<Command>> m_undoStack;
    std::vector<std::unique_ptr<Command>> m_redoStack;
    static constexpr size_t MAX_HISTORY = 100;
};

} // namespace PixellentModeler
