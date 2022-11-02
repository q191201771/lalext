package main

import (
	"fmt"
	"log"

	"github.com/pion/logging"
)

type rtcLogger struct{}

// Print all messages except trace
func (c rtcLogger) Trace(msg string) {
	log.Printf("[Pion Info]: %s\n", msg)
}
func (c rtcLogger) Tracef(format string, args ...interface{}) {
	c.Trace(fmt.Sprintf(format, args...))
}

func (c rtcLogger) Debug(msg string) {
	log.Printf("[Pion Debug]: %s\n", msg)
}
func (c rtcLogger) Debugf(format string, args ...interface{}) {
	c.Debug(fmt.Sprintf(format, args...))
}
func (c rtcLogger) Info(msg string) {
	log.Printf("[Pion Info]: %s\n", msg)
}
func (c rtcLogger) Infof(format string, args ...interface{}) {
	c.Info(fmt.Sprintf(format, args...))
}
func (c rtcLogger) Warn(msg string) {
	log.Printf("[Pion Warn]: %s\n", msg)
}
func (c rtcLogger) Warnf(format string, args ...interface{}) {
	c.Warn(fmt.Sprintf(format, args...))
}
func (c rtcLogger) Error(msg string) {
	log.Printf("[Pion Error]: %s\n", msg)
}
func (c rtcLogger) Errorf(format string, args ...interface{}) {
	c.Error(fmt.Sprintf(format, args...))
}

// rtcLoggerFactory satisfies the interface logging.LoggerFactory
// This allows us to create different loggers per subsystem. So we can
// add custom behavior
type rtcLoggerFactory struct{}

func (c rtcLoggerFactory) NewLogger(subsystem string) logging.LeveledLogger {
	return rtcLogger{}
}
