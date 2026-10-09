#pragma once

#include <QObject>

namespace compositor::ui {

// Installs a runtime Simplified-Chinese localizer.
//
// It walks the live widget / action tree and replaces known English UI text
// with Chinese. It never modifies the underlying data model, persistence keys,
// or the application's source string literals, so saved projects and internal
// identifiers stay byte-for-byte compatible with the original English build.
//
// The returned object is owned by `parent` (and thus by the QApplication).
QObject* installSimplifiedChineseLocalizer(QObject* parent);

}  // namespace compositor::ui
