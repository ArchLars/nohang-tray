#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include "NoHangConfig.h"

static QString writeConfig(const QString& content, QTemporaryDir& dir) {
    QString path = dir.path() + "/test.conf";
    QFile f(path);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream ts(&f);
    ts << content;
    f.close();
    return path;
}

TEST(NoHangConfigTest, ParsePercentages) {
    QTemporaryDir dir;
    QString path = writeConfig(
        "warning_threshold_min_mem = 10%\n"
        "soft_threshold_min_mem = 10 %\n", dir);
    NoHangConfig cfg;
    cfg.ensureParsed(path);
    const auto& th = cfg.thresholds();
    ASSERT_TRUE(th.warn_mem_percent.has_value());
    EXPECT_DOUBLE_EQ(*th.warn_mem_percent, 10.0);
    ASSERT_TRUE(th.soft_mem_percent.has_value());
    EXPECT_DOUBLE_EQ(*th.soft_mem_percent, 10.0);
}

TEST(NoHangConfigTest, ParseMiBValues) {
    QTemporaryDir dir;
    QString path = writeConfig(
        "warning_threshold_min_mem = 512M\n"
        "soft_threshold_min_mem = 512 MiB\n", dir);
    NoHangConfig cfg;
    cfg.ensureParsed(path);
    const auto& th = cfg.thresholds();
    ASSERT_TRUE(th.warn_mem_percent.has_value());
    EXPECT_DOUBLE_EQ(*th.warn_mem_percent, 512.0);
    ASSERT_TRUE(th.soft_mem_percent.has_value());
    EXPECT_DOUBLE_EQ(*th.soft_mem_percent, 512.0);
}

TEST(NoHangConfigTest, FallbackToDefaultPaths) {
    QString fallbackDir = "/usr/share/nohang";
    QDir().mkpath(fallbackDir);
    QString fallbackPath = fallbackDir + "/nohang.conf";
    {
        QFile f(fallbackPath);
        f.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream ts(&f);
        ts << "warning_threshold_min_mem = 10%\n";
    }
    NoHangConfig cfg;
    cfg.ensureParsed("/nonexistent/path.conf");
    EXPECT_EQ(cfg.sourcePath(), fallbackPath);
    const auto& th = cfg.thresholds();
    ASSERT_TRUE(th.warn_mem_percent.has_value());
    EXPECT_DOUBLE_EQ(*th.warn_mem_percent, 10.0);
    QFile::remove(fallbackPath);
}

TEST(NoHangConfigTest, CommentAndEmptyLineHandling) {
    QTemporaryDir dir;
    QString path = writeConfig(
        "# comment\n"
        "@ignored\n"
        "\n"
        "warning_threshold_min_mem = 10%\n"
        "\n"
        "soft_threshold_min_mem = 20%\n", dir);
    NoHangConfig cfg;
    cfg.ensureParsed(path);
    const auto& th = cfg.thresholds();
    ASSERT_TRUE(th.warn_mem_percent.has_value());
    EXPECT_DOUBLE_EQ(*th.warn_mem_percent, 10.0);
    ASSERT_TRUE(th.soft_mem_percent.has_value());
    EXPECT_DOUBLE_EQ(*th.soft_mem_percent, 20.0);
}
