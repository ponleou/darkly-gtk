// Isolated replica of kde-gtk-config's ConfigValueProvider::colors()
// Usage: kcolorscheme2gtk <input.color> [output.css]
//
// Opens the passed KDE color-scheme (.color) file, runs it through the real
// KF6 KColorScheme / KColorUtils machinery (exactly like the module does),
// and writes a gtk-3.0 colors.css in the same format the module produces.
#include <iostream>

#include <QColor>
#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QList>
#include <QMap>
#include <QPair>
#include <QString>
#include <QTextStream>

#include <KColorScheme>
#include <KColorUtils>
#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

#include <algorithm>

namespace {
using KCS = KColorScheme;

// Build the same three-state x category collection the module builds.
QHash<QString, QHash<QString, KCS>>
buildCollections(const KSharedConfigPtr &shared) {
  QHash<QString, QHash<QString, KCS>> csc;

  for (const QString &state :
       {QStringLiteral("active"), QStringLiteral("inactive"),
        QStringLiteral("disabled")}) {
    QPalette::ColorGroup qp = QPalette::Active;
    if (state == QStringLiteral("inactive"))
      qp = QPalette::Inactive;
    else if (state == QStringLiteral("disabled"))
      qp = QPalette::Disabled;

    QHash<QString, KCS> cats;
    for (const QString &cat :
         {QStringLiteral("view"), QStringLiteral("window"),
          QStringLiteral("button"), QStringLiteral("selection"),
          QStringLiteral("tooltip"), QStringLiteral("complementary"),
          QStringLiteral("header")}) {
      cats.insert(
          cat,
          KCS(qp,
              static_cast<KCS::ColorSet>(
                  cat == QStringLiteral("view")            ? KCS::View
                  : cat == QStringLiteral("window")        ? KCS::Window
                  : cat == QStringLiteral("button")        ? KCS::Button
                  : cat == QStringLiteral("selection")     ? KCS::Selection
                  : cat == QStringLiteral("tooltip")       ? KCS::Tooltip
                  : cat == QStringLiteral("complementary") ? KCS::Complementary
                                                           : KCS::Header),
              shared));
    }
    csc.insert(state, cats);
  }
  return csc;
}
} // namespace

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <input.color> [output.css]\n";
    return 1;
  }

  const QString colorPath = QString::fromUtf8(argv[1]);
  const QString outPath =
      (argc > 2) ? QString::fromUtf8(argv[2]) : QStringLiteral("colors.css");

  // Point KColorScheme at our file instead of the session's active scheme.
  auto shared = KSharedConfig::openConfig(colorPath);
  KConfigGroup kdeglobalsConfig = KConfigGroup(shared.data(), QString());

  const qreal bias = KColorScheme::frameContrast();

  const auto csc = buildCollections(shared);

  // ---- Color mixing (verbatim from the module) ----
  QColor windowForegroundColor =
      csc["active"]["window"].foreground(KCS::NormalText).color();
  QColor windowBackgroundColor =
      csc["active"]["window"].background(KCS::NormalBackground).color();
  QColor bordersColor =
      KColorUtils::mix(windowBackgroundColor, windowForegroundColor, bias);

  QColor inactiveWindowForegroundColor =
      csc["inactive"]["window"].foreground(KCS::NormalText).color();
  QColor inactiveWindowBackgroundColor =
      csc["inactive"]["window"].background(KCS::NormalBackground).color();
  QColor inactiveBordersColor = KColorUtils::mix(
      inactiveWindowBackgroundColor, inactiveWindowForegroundColor, bias);

  QColor disabledWindowForegroundColor =
      csc["disabled"]["window"].foreground(KCS::NormalText).color();
  QColor disabledWindowBackgroundColor =
      csc["disabled"]["window"].background(KCS::NormalBackground).color();
  QColor disabledBordersColor = KColorUtils::mix(
      disabledWindowBackgroundColor, disabledWindowForegroundColor, bias);

  QColor unfocusedDisabledWindowForegroundColor =
      csc["disabled"]["window"].foreground(KCS::NormalText).color();
  QColor unfocusedDisabledWindowBackgroundColor =
      csc["disabled"]["window"].background(KCS::NormalBackground).color();
  QColor unfocusedDisabledBordersColor =
      KColorUtils::mix(unfocusedDisabledWindowBackgroundColor,
                       unfocusedDisabledWindowForegroundColor, bias);

  QColor tooltipForegroundColor =
      csc["active"]["tooltip"].foreground(KCS::NormalText).color();
  QColor tooltipBackgroundColor =
      csc["active"]["tooltip"].background(KCS::NormalBackground).color();
  QColor tooltipBorderColor =
      KColorUtils::mix(tooltipBackgroundColor, tooltipForegroundColor, bias);

  KConfigGroup windowManagerConfig =
      KConfigGroup(shared.data(), QStringLiteral("WM"));

  QMap<QString, QColor> result = {
      /* Normal (Non Backdrop, Non Insensitive) */
      {"theme_fg_color_breeze",
       csc["active"]["window"].foreground(KCS::NormalText).color()},
      {"theme_bg_color_breeze",
       csc["active"]["window"].background(KCS::NormalBackground).color()},
      {"theme_text_color_breeze",
       csc["active"]["view"].foreground(KCS::NormalText).color()},
      {"theme_base_color_breeze",
       csc["active"]["view"].background(KCS::NormalBackground).color()},
      {"theme_view_hover_decoration_color_breeze",
       csc["active"]["view"].decoration(KCS::HoverColor).color()},
      {"theme_hovering_selected_bg_color_breeze",
       csc["active"]["selection"].decoration(KCS::HoverColor).color()},
      {"theme_selected_bg_color_breeze",
       csc["active"]["selection"].background(KCS::NormalBackground).color()},
      {"theme_selected_fg_color_breeze",
       csc["active"]["selection"].foreground(KCS::NormalText).color()},
      {"theme_view_active_decoration_color_breeze",
       csc["active"]["view"].decoration(KCS::HoverColor).color()},

      {"theme_button_background_normal_breeze",
       csc["active"]["button"].background(KCS::NormalBackground).color()},
      {"theme_button_decoration_hover_breeze",
       csc["active"]["button"].decoration(KCS::HoverColor).color()},
      {"theme_button_decoration_focus_breeze",
       csc["active"]["button"].decoration(KCS::FocusColor).color()},
      {"theme_button_foreground_normal_breeze",
       csc["active"]["button"].foreground(KCS::NormalText).color()},
      {"theme_button_foreground_active_breeze",
       csc["active"]["selection"].foreground(KCS::NormalText).color()},

      {"borders_breeze", bordersColor},
      {"warning_color_breeze",
       csc["active"]["view"].foreground(KCS::NeutralText).color()},
      {"success_color_breeze",
       csc["active"]["view"].foreground(KCS::PositiveText).color()},
      {"error_color_breeze",
       csc["active"]["view"].foreground(KCS::NegativeText).color()},

      /* Backdrop (Inactive) */
      {"theme_unfocused_fg_color_breeze",
       csc["inactive"]["window"].foreground(KCS::NormalText).color()},
      {"theme_unfocused_text_color_breeze",
       csc["inactive"]["view"].foreground(KCS::NormalText).color()},
      {"theme_unfocused_bg_color_breeze",
       csc["inactive"]["window"].background(KCS::NormalBackground).color()},
      {"theme_unfocused_base_color_breeze",
       csc["inactive"]["view"].background(KCS::NormalBackground).color()},
      {"theme_unfocused_selected_bg_color_alt_breeze",
       csc["inactive"]["selection"].background(KCS::NormalBackground).color()},
      {"theme_unfocused_selected_bg_color_breeze",
       csc["inactive"]["selection"].background(KCS::NormalBackground).color()},
      {"theme_unfocused_selected_fg_color_breeze",
       csc["inactive"]["selection"].foreground(KCS::NormalText).color()},

      {"theme_button_background_backdrop_breeze",
       csc["inactive"]["button"].background(KCS::NormalBackground).color()},
      {"theme_button_decoration_hover_backdrop_breeze",
       csc["inactive"]["button"].decoration(KCS::HoverColor).color()},
      {"theme_button_decoration_focus_backdrop_breeze",
       csc["inactive"]["button"].decoration(KCS::FocusColor).color()},
      {"theme_button_foreground_backdrop_breeze",
       csc["inactive"]["button"].foreground(KCS::NormalText).color()},
      {"theme_button_foreground_active_backdrop_breeze",
       csc["inactive"]["selection"].foreground(KCS::NormalText).color()},

      {"unfocused_borders_breeze", inactiveBordersColor},
      {"warning_color_backdrop_breeze",
       csc["inactive"]["view"].foreground(KCS::NeutralText).color()},
      {"success_color_backdrop_breeze",
       csc["inactive"]["view"].foreground(KCS::PositiveText).color()},
      {"error_color_backdrop_breeze",
       csc["inactive"]["view"].foreground(KCS::NegativeText).color()},

      /* Insensitive (Disabled) */
      {"insensitive_fg_color_breeze",
       csc["disabled"]["window"].foreground(KCS::NormalText).color()},
      {"insensitive_base_fg_color_breeze",
       csc["disabled"]["view"].foreground(KCS::NormalText).color()},
      {"insensitive_bg_color_breeze",
       csc["disabled"]["window"].background(KCS::NormalBackground).color()},
      {"insensitive_base_color_breeze",
       csc["disabled"]["view"].background(KCS::NormalBackground).color()},
      {"insensitive_selected_bg_color_breeze",
       csc["disabled"]["selection"].background(KCS::NormalBackground).color()},
      {"insensitive_selected_fg_color_breeze",
       csc["disabled"]["selection"].foreground(KCS::NormalText).color()},

      {"theme_button_background_insensitive_breeze",
       csc["disabled"]["button"].background(KCS::NormalBackground).color()},
      {"theme_button_decoration_hover_insensitive_breeze",
       csc["disabled"]["button"].decoration(KCS::HoverColor).color()},
      {"theme_button_decoration_focus_insensitive_breeze",
       csc["disabled"]["button"].decoration(KCS::FocusColor).color()},
      {"theme_button_foreground_insensitive_breeze",
       csc["disabled"]["button"].foreground(KCS::NormalText).color()},
      {"theme_button_foreground_active_insensitive_breeze",
       csc["disabled"]["selection"].foreground(KCS::NormalText).color()},

      {"insensitive_borders_breeze", disabledBordersColor},
      {"warning_color_insensitive_breeze",
       csc["disabled"]["view"].foreground(KCS::NeutralText).color()},
      {"success_color_insensitive_breeze",
       csc["disabled"]["view"].foreground(KCS::PositiveText).color()},
      {"error_color_insensitive_breeze",
       csc["disabled"]["view"].foreground(KCS::NegativeText).color()},

      /* Insensitive Backdrop (Inactive Disabled) */
      {"insensitive_unfocused_fg_color_breeze",
       csc["disabled"]["window"].foreground(KCS::NormalText).color()},
      {"theme_unfocused_view_text_color_breeze",
       csc["disabled"]["view"].foreground(KCS::NormalText).color()},
      {"insensitive_unfocused_bg_color_breeze",
       csc["disabled"]["window"].background(KCS::NormalBackground).color()},
      {"theme_unfocused_view_bg_color_breeze",
       csc["disabled"]["view"].background(KCS::NormalBackground).color()},
      {"insensitive_unfocused_selected_bg_color_breeze",
       csc["disabled"]["selection"].background(KCS::NormalBackground).color()},
      {"insensitive_unfocused_selected_fg_color_breeze",
       csc["disabled"]["selection"].foreground(KCS::NormalText).color()},

      {"theme_button_background_backdrop_insensitive_breeze",
       csc["disabled"]["button"].background(KCS::NormalBackground).color()},
      {"theme_button_decoration_hover_backdrop_insensitive_breeze",
       csc["disabled"]["button"].decoration(KCS::HoverColor).color()},
      {"theme_button_decoration_focus_backdrop_insensitive_breeze",
       csc["disabled"]["button"].decoration(KCS::FocusColor).color()},
      {"theme_button_foreground_backdrop_insensitive_breeze",
       csc["disabled"]["button"].foreground(KCS::NormalText).color()},
      {"theme_button_foreground_active_backdrop_insensitive_breeze",
       csc["disabled"]["selection"].foreground(KCS::NormalText).color()},

      {"unfocused_insensitive_borders_breeze", unfocusedDisabledBordersColor},
      {"warning_color_insensitive_backdrop_breeze",
       csc["disabled"]["view"].foreground(KCS::NeutralText).color()},
      {"success_color_insensitive_backdrop_breeze",
       csc["disabled"]["view"].foreground(KCS::PositiveText).color()},
      {"error_color_insensitive_backdrop_breeze",
       csc["disabled"]["view"].foreground(KCS::NegativeText).color()},

      /* Ignorant Colors */
      {"link_color_breeze",
       csc["active"]["view"].foreground(KCS::LinkText).color()},
      {"link_visited_color_breeze",
       csc["active"]["view"].foreground(KCS::VisitedText).color()},

      {"tooltip_text_breeze", tooltipForegroundColor},
      {"tooltip_background_breeze", tooltipBackgroundColor},
      {"tooltip_border_breeze", tooltipBorderColor},

      {"content_view_bg_breeze",
       csc["active"]["view"].background(KCS::NormalBackground).color()},
  };

  // Headers / titlebars
  if (KColorScheme::isColorSetSupported(shared, KCS::Header)) {
    result.insert({{"theme_header_background_breeze",
                    csc["active"]["header"].background().color()},
                   {"theme_header_foreground_breeze",
                    csc["active"]["header"].foreground().color()},
                   {"theme_header_background_light_breeze",
                    csc["active"]["window"].background().color()},
                   {"theme_header_foreground_backdrop_breeze",
                    csc["inactive"]["header"].foreground().color()},
                   {"theme_header_background_backdrop_breeze",
                    csc["inactive"]["header"].background().color()},
                   {"theme_header_foreground_insensitive_breeze",
                    csc["inactive"]["header"].foreground().color()},
                   {"theme_header_foreground_insensitive_backdrop_breeze",
                    csc["inactive"]["header"].foreground().color()},

                   {"theme_titlebar_background_breeze",
                    csc["active"]["header"].background().color()},
                   {"theme_titlebar_foreground_breeze",
                    csc["active"]["header"].foreground().color()},
                   {"theme_titlebar_background_light_breeze",
                    csc["active"]["window"].background().color()},
                   {"theme_titlebar_foreground_backdrop_breeze",
                    csc["inactive"]["header"].foreground().color()},
                   {"theme_titlebar_background_backdrop_breeze",
                    csc["inactive"]["header"].background().color()},
                   {"theme_titlebar_foreground_insensitive_breeze",
                    csc["inactive"]["header"].foreground().color()},
                   {"theme_titlebar_foreground_insensitive_backdrop_breeze",
                    csc["inactive"]["header"].foreground().color()}});
  } else {
    result.insert({
        {"theme_header_background_breeze",
         csc["active"]["window"].background().color()},
        {"theme_header_foreground_breeze",
         csc["active"]["window"].foreground().color()},
        {"theme_header_background_light_breeze",
         csc["active"]["window"].background().color()},
        {"theme_header_foreground_backdrop_breeze",
         csc["inactive"]["window"].foreground().color()},
        {"theme_header_background_backdrop_breeze",
         csc["inactive"]["window"].background().color()},
        {"theme_header_foreground_insensitive_breeze",
         csc["inactive"]["window"].foreground().color()},
        {"theme_header_foreground_insensitive_backdrop_breeze",
         csc["inactive"]["window"].foreground().color()},

        {"theme_titlebar_background_breeze",
         windowManagerConfig.readEntry("activeBackground", QColor())},
        {"theme_titlebar_foreground_breeze",
         windowManagerConfig.readEntry("activeForeground", QColor())},
        {"theme_titlebar_background_light_breeze",
         csc["active"]["window"].background(KCS::NormalBackground).color()},
        {"theme_titlebar_foreground_backdrop_breeze",
         windowManagerConfig.readEntry("inactiveForeground", QColor())},
        {"theme_titlebar_background_backdrop_breeze",
         windowManagerConfig.readEntry("inactiveBackground", QColor())},
        {"theme_titlebar_foreground_insensitive_breeze",
         windowManagerConfig.readEntry("inactiveForeground", QColor())},
        {"theme_titlebar_foreground_insensitive_backdrop_breeze",
         windowManagerConfig.readEntry("inactiveForeground", QColor())},
    });
  }

  // Write exactly like custom_css.cpp: '@define-color KEY name();\n', QMap
  // iterates sorted by key.
  QFile out(outPath);
  if (!out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
    std::cerr << "Cannot open output file: " << outPath.toStdString() << "\n";
    return 1;
  }
  QTextStream outStream(&out);
  for (auto it = result.cbegin(); it != result.cend(); ++it) {
    outStream << QStringLiteral("@define-color %1 %2;\n")
                     .arg(it.key(), it.value().name());
  }

  // ---- Extended color variables ----
  QColor selectedBgColor =
      csc["active"]["selection"].background(KCS::NormalBackground).color();
  QColor selectedFgColor = 
      csc["active"]["selection"].foreground(KCS::NormalText).color();
  QColor viewForegroundColor =
      csc["active"]["view"].foreground(KCS::NormalText).color();
  QColor viewBaseColor =
      csc["active"]["view"].background(KCS::NormalBackground).color();
  QColor headerBackgroundColor =
      csc["active"]["header"].background(KCS::NormalBackground).color();
  QColor headerBackdropBackgroundColor =
      csc["inactive"]["header"].background(KCS::NormalBackground).color();
  QColor errorColor =
      csc["active"]["view"].foreground(KCS::NegativeText).color();
  QColor successColor =
      csc["active"]["view"].foreground(KCS::PositiveText).color();
  QColor warningColor =
      csc["active"]["view"].foreground(KCS::NeutralText).color();

  QList<QPair<QString, QColor>> extended = {
      {"accent_color_breeze", selectedBgColor},                             // darkly: --accent-color: $selected_bg_color
      {"accent_bg_color_breeze", selectedBgColor},                          // darkly: --accent-bg-color: var(--accent-color)
      {"accent_fg_color_breeze", selectedFgColor},                          // darkly: $selected-fg-color
      {"destructive_bg_color_breeze", errorColor},                          // darkly: --destructive-bg-color: $destructive-color ($error_color)
      {"destructive_fg_color_breeze", selectedFgColor},                     // darkly: $selected-fg-color
      {"success_bg_color_breeze", successColor},                            // darkly: --success-bg-color: $success_color
      {"success_fg_color_breeze", selectedFgColor},                         // darkly: $selected-fg-color
      {"warning_bg_color_breeze", warningColor},                            // darkly: --warning-bg-color: $warning_color
      {"warning_fg_color_breeze", selectedFgColor},                         // darkly: $selected-fg-color
      {"error_bg_color_breeze", errorColor},                                // darkly: --error-bg-color: $error_color
      {"error_fg_color_breeze", selectedFgColor},                           // darkly: $selected-fg-color
      {"window_fg_color_breeze", windowForegroundColor},                    // darkly: --window-fg-color: $fg_color
      {"view_bg_color_breeze", viewBaseColor},                              // darkly: view bg = $base_color
      {"view_fg_color_breeze", viewForegroundColor},                        // darkly: view fg = $text_color
      {"headerbar_border_color_breeze", bordersColor},                      // derived: darkly borders for the headerbar edge
      {"headerbar_backdrop_color_breeze", headerBackdropBackgroundColor},   // darkly: --headerbar-backdrop-color: $backdrop_titlebar_bg_color 
      {"headerbar_shade_color_breeze", headerBackgroundColor},              // darkly: --headerbar-shade-color: @headerbar_bg_color ($titlebar_bg)
      {"popover_bg_color_breeze", windowBackgroundColor},                   // darkly: $popover_bg_color: $bg_color
      {"popover_fg_color_breeze", windowForegroundColor},                   // derived: popover fg = window fg (darkly does not set it)
      {"scrollbar_outline_color_breeze", bordersColor},                     // derived: darkly borders (darkly does not set it)
  };
  outStream << "\n";
  outStream << "/* Extended color variables for GTK apps compatibility */\n";
  for (const auto &entry : extended) {
    outStream << QStringLiteral("@define-color %1 %2;\n")
                     .arg(entry.first, entry.second.name());
  }
  return 0;
}
