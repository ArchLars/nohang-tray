#include <gtest/gtest.h>
#include "NoHangConfig.h"
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QDir>

class NoHangConfigTest : public ::testing::Test {
protected:
    void writeConfig(const QString& path, const QString& content) {
        QFile f(path);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream ts(&f);
        ts << content;
    }
};

TEST_F(NoHangConfigTest, ParsePercentages) {
    QTemporaryDir dir;
    QString cfgPath = dir.filePath("config.conf");
    writeConfig(cfgPath, "warning_threshold_min_mem=10%\nwarning_threshold_min_swap=10 %\n");
    NoHangConfig cfg;
    cfg.ensureParsed(cfgPath);
    const auto& t = cfg.thresholds();
    ASSERT_TRUE(t.warn_mem_percent.has_value());
    EXPECT_DOUBLE_EQ(10.0, t.warn_mem_percent.value());
    ASSERT_TRUE(t.warn_swap_percent_free.has_value());
    EXPECT_DOUBLE_EQ(10.0, t.warn_swap_percent_free.value());
}

TEST_F(NoHangConfigTest, ParseMiBValues) {
    QTemporaryDir dir;
    QString cfgPath = dir.filePath("config.conf");
    writeConfig(cfgPath, "warning_threshold_min_mem=512M\nwarning_threshold_min_swap=512 MiB\n");
    NoHangConfig cfg;
    cfg.ensureParsed(cfgPath);
    const auto& t = cfg.thresholds();
    ASSERT_TRUE(t.warn_mem_percent.has_value());
    EXPECT_DOUBLE_EQ(512.0, t.warn_mem_percent.value());
    ASSERT_TRUE(t.warn_swap_percent_free.has_value());
    EXPECT_DOUBLE_EQ(512.0, t.warn_swap_percent_free.value());
}

TEST_F(NoHangConfigTest, FallbackToDefaultPaths) {
    // Ensure default config exists
    QString defaultDir = "/usr/share/nohang";
    QDir().mkpath(defaultDir);
    QString defaultPath = defaultDir + "/nohang.conf";
    writeConfig(defaultPath, "hard_threshold_min_mem=10%\n");

    NoHangConfig cfg;
    cfg.ensureParsed("/nonexistent/path.conf");
    EXPECT_EQ(defaultPath, cfg.sourcePath());
    const auto& t = cfg.thresholds();
    ASSERT_TRUE(t.hard_mem_percent.has_value());
    EXPECT_DOUBLE_EQ(10.0, t.hard_mem_percent.value());

    QFile::remove(defaultPath);
}

TEST_F(NoHangConfigTest, IgnoreCommentsAndEmptyLines) {
    QTemporaryDir dir;
    QString cfgPath = dir.filePath("config.conf");
    writeConfig(cfgPath, "# comment\n\nsoft_threshold_min_swap=10%\n@ignored\n");
    NoHangConfig cfg;
    cfg.ensureParsed(cfgPath);
    const auto& t = cfg.thresholds();
    ASSERT_TRUE(t.soft_swap_percent_free.has_value());
    EXPECT_DOUBLE_EQ(10.0, t.soft_swap_percent_free.value());
    EXPECT_FALSE(t.warn_mem_percent.has_value());
}

