// This file is distributed under GNU GPLv3 license. For full license text, see <project-git-repository-root-folder>/LICENSE.md.

#include "common/miscellaneous/localization.h"

#include "saurus/miscellaneous/application.h"

#include <QDir>
#include <QFileInfoList>
#include <QLocale>
#include <QTranslator>

Localization::Localization(QObject* parent)
  : QObject(parent) {}

QString Localization::desiredLanguage() const {
  return qApp->settings()->value(GROUP(General), SETTING(General::Language)).toString();
}

void Localization::loadActiveLanguage() {
  QTranslator* qt_translator = new QTranslator(qApp);
  QTranslator* app_translator = new QTranslator(qApp);
  QString desired_localization = desiredLanguage();

  qDebug("Starting to load active localization. Desired localization is '%s'.", qPrintable(desired_localization));

  if (app_translator->load(QLocale(desired_localization), "textosaurus", QSL("_"), APP_LANG_PATH)) {
    const QString real_loaded_locale = app_translator->translate("QObject", "LANG_ABBREV");

    Application::installTranslator(app_translator);
    qDebug("Application localization '%s' loaded successfully, specifically sublocalization '%s' was loaded.",
           qPrintable(desired_localization),
           qPrintable(real_loaded_locale));
    desired_localization = real_loaded_locale;
  }
  else {
    qWarning("Application localization '%s' was not loaded. Loading '%s' instead.",
             qPrintable(desired_localization),
             DEFAULT_LOCALE);
    desired_localization = DEFAULT_LOCALE;
  }

  if (qt_translator->load(QLocale(desired_localization), "qtbase", QSL("_"), APP_LANG_PATH)) {
    Application::installTranslator(qt_translator);
    qDebug("Qt localization '%s' loaded successfully.", qPrintable(desired_localization));
  }
  else {
    qWarning("Qt localization '%s' was not loaded.", qPrintable(desired_localization));
  }

  m_loadedLanguage = desired_localization;
  m_loadedLocale = QLocale(desired_localization);
  QLocale::setDefault(m_loadedLocale);
}

QList<Language> Localization::installedLanguages() const {
  QList<Language> languages;
  const QDir file_dir(APP_LANG_PATH);

  // Iterate all found language files.
  foreach (const QFileInfo& file, file_dir.entryInfoList(QStringList() << "textosaurus_*.qm", QDir::Files, QDir::Name)) {
    QTranslator translator;

    if (translator.load(file.absoluteFilePath())) {
      Language new_language;

      new_language.m_code = translator.translate("QObject", "LANG_ABBREV");
      new_language.m_name = QLocale(new_language.m_code).nativeLanguageName();
      languages << new_language;
    }
  }

  return languages;
}

QString Localization::loadedLanguage() const {
  return m_loadedLanguage;
}

QLocale Localization::loadedLocale() const {
  return m_loadedLocale;
}
