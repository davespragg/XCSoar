/*
 * Copyright (C) 2012-2017 Max Kellermann <max.kellermann@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SOCKET_DESCRIPTOR_HXX
#define SOCKET_DESCRIPTOR_HXX

#include "OS/FileDescriptor.hxx"

#include <type_traits>

class SocketAddress;
class StaticSocketAddress;

/**
 * An OO wrapper for a UNIX socket descriptor.
 */
class SocketDescriptor : protected FileDescriptor {
protected:
	explicit constexpr SocketDescriptor(FileDescriptor _fd) noexcept
		:FileDescriptor(_fd) {}

public:
	SocketDescriptor() = default;

	explicit constexpr SocketDescriptor(int _fd) noexcept
		:FileDescriptor(_fd) {}

	constexpr bool operator==(SocketDescriptor other) const noexcept {
		return fd == other.fd;
	}

#ifndef _WIN32
	/**
	 * Convert a #FileDescriptor to a #SocketDescriptor.  This is only
	 * possible on operating systems where socket descriptors are the
	 * same as file descriptors (i.e. not on Windows).  Use this only
	 * when you know what you're doing.
	 */
	static constexpr SocketDescriptor FromFileDescriptor(FileDescriptor fd) noexcept {
		return SocketDescriptor(fd);
	}

	/**
	 * Convert this object to a #FileDescriptor instance.  This is only
	 * possible on operating systems where socket descriptors are the
	 * same as file descriptors (i.e. not on Windows).  Use this only
	 * when you know what you're doing.
	 */
	constexpr const FileDescriptor &ToFileDescriptor() const noexcept {
		return *this;
	}
#endif

	using FileDescriptor::IsDefined;
#ifndef _WIN32
	using FileDescriptor::IsValid;
	using FileDescriptor::IsSocket;
#endif

	/**
	 * Determine the socket type, i.e. SOCK_STREAM, SOCK_DGRAM or
	 * SOCK_SEQPACKET.  Returns -1 on error.
	 */
	gcc_pure
	int GetType() const noexcept;

	/**
	 * Is this a stream socket?
	 */
	gcc_pure
	bool IsStream() const noexcept;

	using FileDescriptor::Get;
	using FileDescriptor::Set;
	using FileDescriptor::Steal;
	using FileDescriptor::SetUndefined;

	static constexpr SocketDescriptor Undefined() noexcept {
		return SocketDescriptor(FileDescriptor::Undefined());
	}

	using FileDescriptor::EnableCloseOnExec;
	using FileDescriptor::DisableCloseOnExec;

#ifndef _WIN32
	using FileDescriptor::SetNonBlocking;
	using FileDescriptor::SetBlocking;
	using FileDescriptor::Duplicate;
	using FileDescriptor::Close;
#else
	bool SetNonBlocking() noexcept;

	/**
	 * This method replaces FileDescriptor::Close(), using closesocket()
	 * on Windows.  FileDescriptor::Close() is not virtual, so be
	 * careful when dealing with a FileDescriptor reference that is
	 * really a SocketDescriptor.
	 */
	void Close() noexcept;
#endif

	/**
	 * Create a socket.
	 *
	 * @param domain is the address domain
	 * @param type is the sochet type
	 * @param protocol is the protocol
	 * @return True on success, False on failure
	 * See man 2 socket for detailed information
	 */
	bool Create(int domain, int type, int protocol) noexcept;

	bool Bind(SocketAddress address) noexcept;

	/**
	 * Binds the socket to the port on INADDR_ANY
	 * @param port is the port to bound
	 * @return True on success False on failure
	 */
	bool BindPort(unsigned port) noexcept;

#ifdef __linux__
	/**
	 * Binds the socket to a unique abstract address.
	 */
	bool AutoBind() noexcept;
#endif

	SocketDescriptor Accept() noexcept;

	bool Connect(SocketAddress address) noexcept;

	gcc_pure
	StaticSocketAddress GetLocalAddress() const noexcept;

	gcc_pure
	StaticSocketAddress GetPeerAddress() const noexcept;

	ssize_t Read(void *buffer, size_t length) noexcept;
	ssize_t Write(const void *buffer, size_t length) noexcept;

#ifdef _WIN32
	int WaitReadable(int timeout_ms) const noexcept;
	int WaitWritable(int timeout_ms) const noexcept;
#else
	using FileDescriptor::WaitReadable;
	using FileDescriptor::WaitWritable;
	using FileDescriptor::IsReadyForWriting;
#endif

	/**
	 * Receive a datagram and return the source address.
	 */
	ssize_t Read(void *buffer, size_t length,
		     StaticSocketAddress &address) noexcept;

	/**
	 * Send a datagram to the specified address.
	 */
	ssize_t Write(const void *buffer, size_t length,
		      SocketAddress address) noexcept;

#ifndef _WIN32
	void Shutdown() noexcept;
	void ShutdownRead() noexcept;
	void ShutdownWrite() noexcept;
#endif
};

static_assert(std::is_trivial<SocketDescriptor>::value, "type is not trivial");

#endif